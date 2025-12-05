@echo off

chcp 65001

if exist template.pdf del template.pdf

:: 使用 xelatex 编译 .tex 文件（第一次编译，生成目录文件）
echo 第一次编译，生成目录文件...
xelatex -interaction=nonstopmode template.tex

:: 第二次编译，读取目录文件并生成完整PDF
echo 第二次编译，生成完整PDF...
xelatex -interaction=nonstopmode template.tex

:: 删除中间文件（保留PDF）
del *.aux *.log *.out *.toc *.gz 2>nul

echo 编译完成！
pause
