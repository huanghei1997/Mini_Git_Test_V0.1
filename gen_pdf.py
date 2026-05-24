"""Generate PDF report from Mini-Git_Report.md using fpdf2."""
import markdown
from fpdf import FPDF
from pathlib import Path
import re

md_path = Path(__file__).parent / "Mini-Git_Report.md"
out_path = Path(__file__).parent / "Mini-Git_Report.pdf"

md_text = md_path.read_text(encoding="utf-8")

# Replace all non-latin1 characters with ASCII equivalents
replacements = {
    "\u2014": "--", "\u2013": "-",
    "\u2018": "'", "\u2019": "'",
    "\u201c": '"', "\u201d": '"',
    "\u2192": "->", "\u2190": "<-",
    "\u2191": "^", "\u2193": "v",
    "\u251c": "|--", "\u2514": "`--",
    "\u2502": "|", "\u2500": "-",
    "\u250c": "+--", "\u2510": "--+",
    "\u2518": "--+", "\u2524": "--|",
    "\u252c": "-+-", "\u2534": "-+-",
    "\u253c": "-+-",
    "\u2560": "|==", "\u2550": "=",
    "\u255a": "`==", "\u2554": "+==",
    "\u2563": "==|",
}
for old, new in replacements.items():
    md_text = md_text.replace(old, new)

# Strip any remaining non-latin1 chars
md_text = md_text.encode("latin-1", errors="replace").decode("latin-1")

# Convert markdown to HTML
html = markdown.markdown(md_text, extensions=["tables", "fenced_code"])

# Build PDF
pdf = FPDF()
pdf.set_auto_page_break(auto=True, margin=15)
pdf.add_page()
pdf.set_font("Helvetica", size=10)

pdf.write_html(html)

pdf.output(str(out_path))
print(f"PDF generated: {out_path}")
