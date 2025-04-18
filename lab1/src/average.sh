if [$# -eq 0 ]; then
    echo "Нет входных аргументов"
    exit 1
fi

count=$#
sum=0

for num in "$@"; do
    sum=$((sum + num))
done

average=$(echo "scale=2; $sum / $count" | bc)

echo "Количество аргументов: $count"
echo "Среднее арифметическое: $average"