#!/bin/bash

# number of games to play
N=${1:-10}
PLAYER=${2:-1}
P1_WINS=0
P2_WINS=0
TIES=0

AI_CPP="../src/checkers-cpp/main"
AI_OPP="Sample_AIs/Poor_AI/main.py"
for ((i=1; i<=N; i++))
do
    echo "Running game $i..."
    if [ "$PLAYER" -eq "1" ]; then
        OUTPUT=$(python3 AI_Runner.py 8 8 3 l "$AI_CPP" "$AI_OPP")
    else
        OUTPUT=$(python3 AI_Runner.py 8 8 3 l "$AI_OPP" "$AI_CPP")
    fi

    if echo "$OUTPUT" | grep -q "exception"; then
        echo "EXCEPTION"
    elif echo "$OUTPUT" | grep -q "player 1 wins"; then
        ((P1_WINS++))
        echo "Result: Player 1 wins"
    elif echo "$OUTPUT" | grep -q "player 2 wins"; then
        ((P2_WINS++))
        echo "Result: Player 2 wins"
    elif echo "$OUTPUT" | grep -q "Tie"; then
        ((TIES++))
        echo "Result: Tie"
    else
        echo "⚠️ Unknown result or crash."
    fi
done

if [ "$PLAYER" -eq "1" ]; then
    WINRATE=$(awk "BEGIN {printf \"%.2f\", (($P1_WINS+$TIES)/$N)*100}")
else
    WINRATE=$(awk "BEGIN {printf \"%.2f\", (($P2_WINS+$TIES)/$N)*100}")
fi

# Compute and print win rate
echo
echo "==================== Summary ===================="
echo "Player 1 wins: $P1_WINS"
echo "Player 2 wins: $P2_WINS"
echo "Ties:          $TIES"
echo "Total games:   $N"
echo "-------------------------------------------------"
echo "Player $PLAYER win rate: $WINRATE%"
echo "================================================="


