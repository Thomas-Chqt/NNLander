---
marp: true
theme: default
paginate: true
style: |
  section {
    padding-top: 20px !important;
    padding-bottom: 20px !important;
  }
  .split {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 1rem;
    height: 100%;
  }
  .split > * {
    margin: 0;
  }
---

<!-- _class: centered -->

# NNLander

## Minimal Neural Networks for Control Systems
## 制御システムのための最小限のニューラルネットワーク

> An alternative introduction to Neural Networks
> ニューラルネットワークへの別のアプローチ

<br>
<br>

<div style="text-align: right; font-size: 0.8em;">by Davide Pasca</div>
<div style="text-align: right; font-size: 0.8em;">v 1.0</div>

<div style="position: absolute; bottom: 20px; left: 0; right: 0; text-align: center; font-size: 0.8em;">
<a href="https://github.com/dpasca/NNLander">https://github.com/dpasca/NNLander</a>
</div>

---

# Quickstart

| | | |
|---|---|---------------|
| 1 | See the requirements | [English](workshop_requirements_nn_en.txt) or [日本語](workshop_requirements_nn_ja.txt) |
| 2 | Clone this repo | `git clone git@github.com:dpasca/NNLander.git` |
| 3 | Launch the build | `./build.sh` or `build.bat` |
| 4 | Run the demos | `./build/bin/Lander01` or<br>`build\bin\Release\Lander01.exe` |

---

# Goal of this workshop

We aim to demistify NNs by looking at how they work in practice.
このワークショップの目標は、実際にどのように動作するかを見てニューラルネットワークを解明することです。

Actual NNs fundamentals are interesting but confusing !
実際のニューラルネットワークの基礎は興味深いですが、混乱を招きます！

It will still be confusing, it takes time to truly understand something.
それでも混乱するでしょうが、何かを本当に理解するには時間がかかります。

---

# The Lander Simulation/Game

<div class="split">
<div>
An excellent way to apply NNs. A well defined problem with a set of Inputs and
expected Outputs.

ニューラルネットワークを適用する優れた方法です。入力と期待される出力が明確に定義された問題です。
</div>
<div>
<img src="images/ins_brain_outs.png" alt="Inputs, Brain, Outputs" style="max-width: 100%; height: auto;">
</div>
</div>

---

# Lander01: Manual Control

### Human Brain Interface

- User sees → user presses keys → simulation changes
  ユーザーが見る → キーを押す → シミュレーションが変わる
- Up arrow: vertical thrust
  上矢印: 垂直推力
- Left/right arrows: horizontal thrust
  左右矢印: 水平推力

Try to land on the pad, softly ! (below speed 1.5)
パッドに着陸してみてください、優しく！（速度1.5以下）

---

# Lander02: Rule-based AI

### Fixed Brain Approach

- Pre-programmed rules based on programmer's observation
  プログラマーの観察に基づいた事前プログラムされたルール
- Simple `if-then` logic for different conditions
  異なる条件に対する単純な`if-then`ロジック
- No learning - just fixed behavior
  学習なし - 固定された動作のみ

---

# Lander02: Rule-based AI - Benefits & Limitations

- **Benefits**: Predictable, explainable behavior, efficient
  **利点**: 予測可能で説明可能な動作、効率的
- **Limitations**:
  - Limited adaptation to new scenarios
    新しいシナリオへの適応が限られている
  - Rules must be manually crafted
    ルールは手動で作成する必要がある
  - Complexity increases with more edge cases
    エッジケースが増えると複雑さが増す

---

# Neural Networks are just equations !

NNs are a fancy way of writing equations.
Think of a set of equations that map inputs ($x_1$, $x_2$, $x_3$, ...) to outputs ($y_1$, $y_2$, $y_3$, ...).

ニューラルネットワークは方程式を書くための洗練された方法です。
入力（$x_1$, $x_2$, $x_3$, ...）を出力（$y_1$, $y_2$, $y_3$, ...）にマッピングする一連の方程式を考えてみて

> $f(x_1 \times w_1 + x_2 \times w_2 + x_3 \times w_3 + ...) = y_1$
> $f(x_1 \times w_4 + x_2 \times w_5 + x_3 \times w_6 + ...) = y_2$

---

# Neural Networks are just equations !

<div class="split">
<div>
The "brain" is in the weights of the equations.

「脳」は方程式の重みにあります。

> $f(x_1 \times w_1 + x_2 \times w_2 + x_3 \times w_3 + ...) = y_1$
> $f(x_1 \times w_4 + x_2 \times w_5 + x_3 \times w_6 + ...) = y_2$

Training is the art of finding the best weights that give desired Outputs for given Inputs.

トレーニングは、与えられた入力に対して望ましい出力を得るための最適な重みを見つける技術です。
</div>
<div>
<div style="display: flex; justify-content: center; align-items: center;">
  <img src="images/simple_nn.svg" alt="Simple NN" style="max-width: 100%; height: auto;">
</div>
</div>

---

# Computer code can be written as equations

## Classical Code (if-then, rule-based)
## 古典的なコード（if-then、ルールベース）
```
if landerX < targetX
  stickPosRight = true
else
  stickPosRight = false
```

## Neural Network
```
stickPosRight = is_positive(w1 * targetX + w2 * landerX)
w1 =  1
w2 = -1
```

---

# Computer code can be written as equations

Assume that our lander's X position is 10 and the target is 15.
我々のランダーのX位置が10で、ターゲットが15であると仮定します。

```cpp
landerX = 10 // Input 1
targetX = 15 // Input 2

w1      =  1 // Weight 1
w2      = -1 // Weight 2

stickPosRight = is_positive(w1 * targetX + w2 * landerX)
stickPosRight = is_positive(1 * 15 + -1 * 10)
stickPosRight = is_positive(15 - 10)
stickPosRight = is_positive(5)
stickPosRight = true // Output !
```

---

# Finally, NNLander with Neural Networks

<img src="../screenshot.png" alt="NNLander Screenshot" width="750">


---

# Lander03: Neural Network Training

### Random Training Approach

- Assing random numbers to all of the weights, and test the simulation.
  すべての重みにランダムな数値を割り当て、シミュレーションをテストします。
- Weights with best score are kept.
  最良のスコアを持つ重みが保持されます。
- Repeat... maybe we get lucky in 10,000 years.
  繰り返します... 10,000年後に運が良ければ。

Highly inefficient "dumb" training.
非常に非効率な「愚かな」トレーニング。

---

# Lander04: Genetic Algorithm Training

Based on natural selection. `-` 自然選択に基づく。

### Population-based Training

- Maintain a population of neural networks
  ニューラルネットワークの集団を維持する
- Use genetic principles: selection, crossover, mutation
  遺伝的原理を使用する：選択、交叉、突然変異
- Evolve better solutions over generations
  世代を重ねてより良い解決策を進化させる

---

# Genetic Algorithm Process

<img src="images/genetic_algorithm.png" alt="Genetic Algorithm Process" width="1200">

---

# Beyond Genetic Algorithm

Backpropagation is a more efficient way to train Neural Networks,
it's the way forward, but comes with its own set of challenges.
逆伝播法はニューラルネットワークを訓練するためのより効率的な方法であり、前進する方法ですが、独自の課題も伴います。

Feel free to explore !
自由に探求してください！

---

# Key Insights & Takeaways

### Neural Networks Can:

- Learn complex behaviors without explicit programming
  明示的なプログラミングなしで複雑な動作を学習する
- Adapt to changing conditions
  変化する条件に適応する
- Solve problems where rules are hard to define
  ルールを定義するのが難しい問題を解決する

---

# Key Insights & Takeaways

### Important Considerations:

- Training method significantly impacts performance
  トレーニング方法はパフォーマンスに大きな影響を与える
- Scoring/fitness function design is critical
  スコアリング/フィットネス関数の設計が重要
- LLMs are helping us today to write classical code, but more and more problems will be solved just with NNs. Start to think in those terms.
  今日、LLMは古典的なコードを書くのを助けていますが、ますます多くの問題がニューラルネットワークだけで解決されるでしょう。そのように考え始めてください。

---

# Thank You!

**Resources:**
- [OpenAI API Documentation](https://platform.openai.com/docs/)
- [Ollama Project](https://ollama.ai/)
- [Node.js Documentation](https://nodejs.org/en/docs/)

**Contact:**

*Davide Pasca*:
- [davide@newtypekk.com](mailto:davide@newtypekk.com)
- [github.com/dpasca](https://github.com/dpasca)
- [newtypekk.com](https://newtypekk.com)
- [x.com/109mae](https://x.com/109mae)

---

# Appendix: Simulation Components

<div style="display: flex; justify-content: center; align-items: center;">
  <img src="images/simulation_components.png" alt="Simulation Components" style="max-width: 90%; height: auto;">
</div>