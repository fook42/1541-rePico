#!/bin/bash

# Static Code Analysis Script
# Führt Cppcheck und clang-tidy lokal aus

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
REPORT_DIR="$PROJECT_ROOT/analysis-reports"

# Farben für Terminal-Output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Hilfsfunktion für Output
print_header() {
    echo -e "${BLUE}=== $1 ===${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

# Nutzung anzeigen
usage() {
    cat << EOF
Static Code Analysis für 1541-rePico

Nutzung: $0 [OPTIONEN]

Optionen:
    --help              Zeige diese Hilfe
    --cppcheck          Nur Cppcheck ausführen
    --clang-tidy        Nur clang-tidy ausführen
    --reports           HTML/XML Reports generieren
    --all               Beide Tools + Reports
    --verbose           Ausführliche Ausgabe

Beispiele:
    $0                  # Beide Tools ausführen
    $0 --all            # Beide Tools + Reports generieren
    $0 --cppcheck       # Nur Cppcheck
    $0 --clang-tidy     # Nur clang-tidy

EOF
    exit 0
}

# Default Werte
RUN_CPPCHECK=true
RUN_CLANG_TIDY=true
GENERATE_REPORTS=false
VERBOSE=false

# Argumente verarbeiten
while [[ $# -gt 0 ]]; do
    case $1 in
        --help)
            usage
            ;;
        --cppcheck)
            RUN_CPPCHECK=true
            RUN_CLANG_TIDY=false
            shift
            ;;
        --clang-tidy)
            RUN_CPPCHECK=false
            RUN_CLANG_TIDY=true
            shift
            ;;
        --reports)
            GENERATE_REPORTS=true
            shift
            ;;
        --all)
            RUN_CPPCHECK=true
            RUN_CLANG_TIDY=true
            GENERATE_REPORTS=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        *)
            print_error "Unbekannte Option: $1"
            usage
            ;;
    esac
done

# In Projektroot wechseln
cd "$PROJECT_ROOT"

# Reports-Verzeichnis erstellen
mkdir -p "$REPORT_DIR"

# Cppcheck ausführen
if [ "$RUN_CPPCHECK" = true ]; then
    print_header "Cppcheck-Analyse"
    
    if ! command -v cppcheck &> /dev/null; then
        print_error "cppcheck ist nicht installiert"
        exit 1
    fi
    
    print_success "cppcheck $(cppcheck --version | grep -oP '(?<=Cppcheck )[^ ]*')"
    
    # Cppcheck ausführen
    if [ "$VERBOSE" = true ]; then
        cppcheck --enable=all --suppress=missingIncludeSystem \
                 --check-level=exhaustive \
                 -I firmware/include \
                 firmware/src firmware/include
    else
        cppcheck --enable=all --suppress=missingIncludeSystem \
                 --output-file="$REPORT_DIR/cppcheck-report.xml" \
                 --xml --xml-version=2 \
                 --check-level=exhaustive \
                 -I firmware/include \
                 firmware/src firmware/include || true
        
        if [ -f "$REPORT_DIR/cppcheck-report.xml" ]; then
            ERROR_COUNT=$(grep -c '<error' "$REPORT_DIR/cppcheck-report.xml" 2>/dev/null || echo 0)
            if [ "$ERROR_COUNT" -gt 0 ]; then
                print_warning "$ERROR_COUNT Probleme gefunden"
            else
                print_success "Keine Probleme gefunden"
            fi
        fi
    fi
    
    echo ""
fi

# clang-tidy ausführen
if [ "$RUN_CLANG_TIDY" = true ]; then
    print_header "clang-tidy-Analyse"
    
    if ! command -v clang-tidy &> /dev/null; then
        print_error "clang-tidy ist nicht installiert"
        exit 1
    fi
    
    if ! command -v cmake &> /dev/null; then
        print_error "cmake ist nicht installiert"
        exit 1
    fi
    
    print_success "clang-tidy $(clang-tidy --version 2>&1 | head -n1)"
    
    # CMake ausführen um compile_commands.json zu generieren
    print_success "Generiere Compilation Database mit CMake..."
    BUILD_DIR="$PROJECT_ROOT/build-analysis"
    mkdir -p "$BUILD_DIR"
    
    cd "$BUILD_DIR"
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE \
          -S "$PROJECT_ROOT/firmware" \
          -B . \
          -G Ninja > /dev/null 2>&1 || true
    cd "$PROJECT_ROOT"
    
    if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
        print_error "compile_commands.json konnte nicht generiert werden"
        echo "Versuche ohne Compilation Database..."
        RUN_WITHOUT_DB=true
    else
        print_success "Compilation Database generiert"
        RUN_WITHOUT_DB=false
    fi
    
    # clang-tidy ausführen
    find firmware/src firmware/include -name "*.c" -type f > /tmp/c_files.txt
    FILE_COUNT=$(wc -l < /tmp/c_files.txt)
    
    if [ "$FILE_COUNT" -eq 0 ]; then
        print_warning "Keine C-Dateien gefunden"
    else
        print_success "$FILE_COUNT C-Dateien gefunden"
        
        if [ "$VERBOSE" = true ]; then
            if [ "$RUN_WITHOUT_DB" = false ]; then
                clang-tidy -p "$BUILD_DIR" $(cat /tmp/c_files.txt) 2>&1 || true
            else
                clang-tidy $(cat /tmp/c_files.txt) 2>&1 || true
            fi
        else
            if [ "$RUN_WITHOUT_DB" = false ]; then
                clang-tidy -p "$BUILD_DIR" $(cat /tmp/c_files.txt) 2>&1 | tee "$REPORT_DIR/clang-tidy-report.txt" || true
            else
                clang-tidy $(cat /tmp/c_files.txt) 2>&1 | tee "$REPORT_DIR/clang-tidy-report.txt" || true
            fi
            
            if [ -f "$REPORT_DIR/clang-tidy-report.txt" ]; then
                # Zähle echte Warnungen (nicht "error generated" oder "file not found")
                ISSUE_COUNT=$(grep -E "warning:|note:" "$REPORT_DIR/clang-tidy-report.txt" 2>/dev/null | wc -l)
                ERROR_COUNT=$(grep "error:" "$REPORT_DIR/clang-tidy-report.txt" 2>/dev/null | grep -v "file not found" | grep -v "error generated" | wc -l)
                
                if [ "$ISSUE_COUNT" -gt 0 ] || [ "$ERROR_COUNT" -gt 0 ]; then
                    print_warning "$ISSUE_COUNT Warnungen, $ERROR_COUNT Fehler gefunden"
                    echo ""
                    echo "Details:"
                    echo "--------"
                    grep -E "^/.*\.(c|h):" "$REPORT_DIR/clang-tidy-report.txt" | head -20
                else
                    print_success "Keine Probleme gefunden"
                fi
            fi
        fi
    fi
    
    echo ""
fi

# HTML-Reports generieren
if [ "$GENERATE_REPORTS" = true ]; then
    print_header "Reports generieren"
    
    if [ -f "$REPORT_DIR/cppcheck-report.xml" ]; then
        print_success "Generiere HTML-Report..."
        python3 "$SCRIPT_DIR/generate_report.py" \
                "$REPORT_DIR/cppcheck-report.xml" \
                "$REPORT_DIR/cppcheck-report.html"
        print_success "Report verfügbar: $REPORT_DIR/cppcheck-report.html"
    fi
    
    echo ""
fi

# Zusammenfassung
print_header "Zusammenfassung"
print_success "Analyse abgeschlossen"
echo ""
echo "Reports verfügbar unter: $REPORT_DIR"
ls -lh "$REPORT_DIR" 2>/dev/null || echo "Keine Reports gefunden"
