// Source MathJax 4.1.3 fb987178c2d279a99b0db5ea02e751691703955e
// Input: pinned direct MathML merror wrapper cases; not TeX input coverage.
// scope-sha256 9705d389538b2fc8bf782feea4372ace639b3727304f2971937bbcfb0042a0d1
// input/mathml.js 8fc5382e4a81b8be02484c9f6de91618ce8a5c859af97b5175df6badf35035db | input/mathml.ts e027607ae93b8dedf7b8312b1b814c50cf8cd0c6b332f56edba02d1d3bd7be1b
// input/mathml/FindMathML.js fec27373d9b959de3511e5162eb98bb50d0b17a95e4e746a62f55fe625b86c2a | input/mathml/FindMathML.ts 1237e59c4a1b1e4852b4bf6986db1cbbb84efc0726426051021bcc6d2595dfd9
// input/mathml/MathMLCompile.js 0b190d56a673941b6b4d1c8e8632dfa894ed50ddccce484e9e8d142c6d1e8880 | input/mathml/MathMLCompile.ts 2bb1f7258daf68fa6d51ea585b08312c1bc407719bd6711651369db1d5b68349
struct MerrorSourceBBox { double width, height, depth, left, right; };
struct MerrorSourceCase { mathml_scene_test::Case source; std::array<MerrorSourceBBox, 2> boxes; std::size_t box_count; };
struct MerrorSourceStyleRule { std::u16string_view selector, fill, stroke; };
constexpr std::array<MerrorSourceStyleRule, 2> sourceMerrorStyleRules{{
  {u"g[data-mml-node=\"merror\"] > g", u"red", u"red"},
  {u"g[data-mml-node=\"merror\"] > rect[data-background]", u"yellow", u"none"}
}};
constexpr std::array<mathml_scene_test::Node, 11> merror0Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"merror", false, 1, 0, 0, u""},
  {"inferredMrow", true, 2, 0, 0, u""},
  {"mrow", false, 3, 0, 0, u""},
  {"mi", false, 4, 0, 0, u""},
  {"text", false, 5, 0, 0, u"x"},
  {"mo", false, 4, 0, 0, u""},
  {"text", false, 7, 0, 0, u"+"},
  {"mi", false, 4, 0, 0, u""},
  {"text", false, 9, 0, 0, u"y"}
}};
constexpr std::array<mathml_scene_test::Attribute, 0> merror0Attributes{{

}};

constexpr std::array<mathml_scene_test::Node, 6> merror1Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"merror", false, 1, 0, 1, u""},
  {"inferredMrow", true, 2, 1, 0, u""},
  {"mi", false, 3, 1, 0, u""},
  {"text", false, 4, 1, 0, u"x"}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> merror1Attributes{{
  {u"title", mathml_scene_test::AttributeKind::text, u"", 0, false}
}};

constexpr std::array<mathml_scene_test::Node, 6> merror2Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"merror", false, 1, 0, 1, u""},
  {"inferredMrow", true, 2, 1, 0, u""},
  {"mi", false, 3, 1, 0, u""},
  {"text", false, 4, 1, 0, u"x"}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> merror2Attributes{{
  {u"title", mathml_scene_test::AttributeKind::text, u"Title & \"quoted\" <tag>", 0, false}
}};

constexpr std::array<mathml_scene_test::Node, 10> merror3Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"merror", false, 1, 0, 0, u""},
  {"inferredMrow", true, 2, 0, 0, u""},
  {"mi", false, 3, 0, 0, u""},
  {"text", false, 4, 0, 0, u"a"},
  {"mo", false, 3, 0, 0, u""},
  {"text", false, 6, 0, 0, u"+"},
  {"mi", false, 3, 0, 0, u""},
  {"text", false, 8, 0, 0, u"b"}
}};
constexpr std::array<mathml_scene_test::Attribute, 0> merror3Attributes{{

}};

constexpr std::array<mathml_scene_test::Node, 16> merror4Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"merror", false, 1, 0, 1, u""},
  {"inferredMrow", true, 2, 1, 0, u""},
  {"mrow", false, 3, 1, 0, u""},
  {"mi", false, 4, 1, 0, u""},
  {"text", false, 5, 1, 0, u"a"},
  {"merror", false, 4, 1, 1, u""},
  {"inferredMrow", true, 7, 2, 0, u""},
  {"mrow", false, 8, 2, 0, u""},
  {"mi", false, 9, 2, 0, u""},
  {"text", false, 10, 2, 0, u"x"},
  {"mo", false, 9, 2, 0, u""},
  {"text", false, 12, 2, 0, u"+"},
  {"mi", false, 9, 2, 0, u""},
  {"text", false, 14, 2, 0, u"y"}
}};
constexpr std::array<mathml_scene_test::Attribute, 2> merror4Attributes{{
  {u"title", mathml_scene_test::AttributeKind::text, u"outer", 0, false},
  {u"title", mathml_scene_test::AttributeKind::text, u"inner", 0, false}
}};

constexpr std::array<mathml_scene_test::Node, 8> merror5Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"merror", false, 1, 0, 0, u""},
  {"inferredMrow", true, 2, 0, 0, u""},
  {"mi", false, 3, 0, 1, u""},
  {"text", false, 4, 1, 0, u"x"},
  {"mi", false, 3, 1, 1, u""},
  {"text", false, 6, 2, 0, u"y"}
}};
constexpr std::array<mathml_scene_test::Attribute, 2> merror5Attributes{{
  {u"mathcolor", mathml_scene_test::AttributeKind::text, u"blue", 0, false},
  {u"mathcolor", mathml_scene_test::AttributeKind::text, u"blue", 0, false}
}};

constexpr std::array<mathml_scene_test::Node, 11> merror6Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"merror", false, 1, 0, 0, u""},
  {"inferredMrow", true, 2, 0, 0, u""},
  {"mrow", false, 3, 0, 1, u""},
  {"mo", false, 4, 1, 2, u""},
  {"text", false, 5, 3, 0, u"("},
  {"mi", false, 4, 3, 0, u""},
  {"text", false, 7, 3, 0, u"x"},
  {"mo", false, 4, 3, 2, u""},
  {"text", false, 9, 5, 0, u")"}
}};
constexpr std::array<mathml_scene_test::Attribute, 5> merror6Attributes{{
  {u"href", mathml_scene_test::AttributeKind::text, u"https://example.test/merror", 0, false},
  {u"stretchy", mathml_scene_test::AttributeKind::boolean, {}, 0, true},
  {u"minsize", mathml_scene_test::AttributeKind::text, u"5em", 0, false},
  {u"stretchy", mathml_scene_test::AttributeKind::boolean, {}, 0, true},
  {u"minsize", mathml_scene_test::AttributeKind::text, u"5em", 0, false}
}};

constexpr MerrorSourceCase sourceMerrorCases[]{
  {mathml_scene_test::Case{"merror-basic-text",mathml_scene_test::Metrics{16,8,640,false},merror0Nodes,merror0Attributes,R"FOLIAMERROR(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.464ex;" xmlns="http://www.w3.org/2000/svg" width="4.163ex" height="1.783ex" role="img" focusable="false" viewBox="0 -583 1840 788"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="merror"><rect data-background="true" width="1840" height="788" y="-205"/><g data-mml-node="mrow"><g data-mml-node="mi"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g><g data-mml-node="mo" transform="translate(572,0)"><path data-c="2B" d="M698 274L413 274L413 559C413 575 405 583 389 583C373 583 365 575 365 559L365 274L80 274C64 274 56 266 56 250C56 234 64 226 80 226L365 226L365-59C365-75 373-83 389-83C405-83 413-75 413-59L413 226L698 226C714 226 722 234 722 250C722 263 711 274 698 274Z"/></g><g data-mml-node="mi" transform="translate(1350,0)"><path data-c="1D466" d="M163 442C118 442 83 417 58 367C39 328 29 301 29 286C29 277 34 272 45 272C59 272 60 278 64 293C87 372 119 412 160 412C173 412 180 403 180 385C180 369 175 346 164 317C126 214 107 145 107 108C107 32 154-13 231-13C264-13 295-1 323 23C288-109 233-175 158-175C124-175 101-164 89-142C129-140 149-121 149-85C149-60 136-47 109-47C72-47 50-78 50-115C50-170 100-205 158-205C273-205 367-99 392-1L486 377C489 388 490 396 490 401C490 421 479 431 458 431C442 431 429 423 420 408C414 387 409 369 406 354L342 97C332 61 279 16 234 16C196 16 177 41 177 92C177 134 194 198 227 284C240 319 247 343 247 357C247 406 212 442 163 442Z"/></g></g></g></g></g></svg></mjx-container>)FOLIAMERROR"},std::array<MerrorSourceBBox, 2>{{{1.84,0.583,0.205,0,0},{}}},1},
  {mathml_scene_test::Case{"merror-empty-title",mathml_scene_test::Metrics{16,8,640,false},merror1Nodes,merror1Attributes,R"FOLIAMERROR(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.025ex;" xmlns="http://www.w3.org/2000/svg" width="1.294ex" height="1.025ex" role="img" focusable="false" viewBox="0 -442 572 453"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="merror" title=""><rect data-background="true" width="572" height="453" y="-11"/><g data-mml-node="mi"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g></g></g></g></svg></mjx-container>)FOLIAMERROR"},std::array<MerrorSourceBBox, 2>{{{0.572,0.442,0.011,0,0},{}}},1},
  {mathml_scene_test::Case{"merror-escaped-title",mathml_scene_test::Metrics{16,8,640,false},merror2Nodes,merror2Attributes,R"FOLIAMERROR(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.025ex;" xmlns="http://www.w3.org/2000/svg" width="1.294ex" height="1.025ex" role="img" focusable="false" viewBox="0 -442 572 453"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="merror" title="Title &amp; &quot;quoted&quot; &lt;tag&gt;"><rect data-background="true" width="572" height="453" y="-11"/><title>Title & "quoted" <tag></title><g data-mml-node="mi"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g></g></g></g></svg></mjx-container>)FOLIAMERROR"},std::array<MerrorSourceBBox, 2>{{{0.572,0.442,0.011,0,0},{}}},1},
  {mathml_scene_test::Case{"merror-multiple-children",mathml_scene_test::Metrics{16,8,640,false},merror3Nodes,merror3Attributes,R"FOLIAMERROR(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.188ex;" xmlns="http://www.w3.org/2000/svg" width="3.928ex" height="1.758ex" role="img" focusable="false" viewBox="0 -694 1736 777"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="merror"><rect data-background="true" width="1736" height="777" y="-83"/><g data-mml-node="mi"><path data-c="1D44E" d="M498 144C498 153 493 158 482 158C474 158 468 151 465 137C445 58 421 18 394 18C377 18 368 32 368 60C368 73 372 97 381 132L438 357C443 376 445 387 445 392C445 412 434 422 412 422C391 422 377 410 370 387C349 424 319 442 281 442C216 442 159 409 109 343C63 281 40 217 40 150C40 63 91-11 175-11C218-11 260 12 300 58C311 20 345-11 392-11C461-11 482 70 498 144M341 374C350 353 355 339 355 330C355 326 354 321 353 314L304 122C301 111 294 99 285 87C248 41 212 18 177 18C138 18 118 48 118 107C118 131 124 167 136 215C157 300 187 357 224 388C244 405 263 413 282 413C309 413 329 400 341 374Z"/></g><g data-mml-node="mo" transform="translate(529,0)"><path data-c="2B" d="M698 274L413 274L413 559C413 575 405 583 389 583C373 583 365 575 365 559L365 274L80 274C64 274 56 266 56 250C56 234 64 226 80 226L365 226L365-59C365-75 373-83 389-83C405-83 413-75 413-59L413 226L698 226C714 226 722 234 722 250C722 263 711 274 698 274Z"/></g><g data-mml-node="mi" transform="translate(1307,0)"><path data-c="1D44F" d="M281 445C245 445 209 428 174 394L243 679C241 688 238 694 226 694C193 694 120 685 107 684C92 682 84 675 84 660C84 650 93 645 112 645C131 645 157 646 157 632C157 628 152 608 143 571L63 249C52 207 47 173 47 148C47 62 93-11 175-11C240-11 297 22 347 89C392 151 415 216 415 283C415 370 364 445 281 445M279 415C318 415 337 385 337 326C337 299 331 263 319 218C297 133 268 75 233 44C213 27 194 19 175 19C134 19 114 51 114 115C114 137 119 170 129 214L151 304C154 319 159 330 165 338C204 389 242 415 279 415Z"/></g></g></g></g></svg></mjx-container>)FOLIAMERROR"},std::array<MerrorSourceBBox, 2>{{{1.736,0.694,0.083,0,0},{}}},1},
  {mathml_scene_test::Case{"merror-nested",mathml_scene_test::Metrics{16,8,640,false},merror4Nodes,merror4Attributes,R"FOLIAMERROR(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.464ex;" xmlns="http://www.w3.org/2000/svg" width="5.36ex" height="1.783ex" role="img" focusable="false" viewBox="0 -583 2369 788"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="merror" title="outer"><rect data-background="true" width="2369" height="788" y="-205"/><title>outer</title><g data-mml-node="mrow"><g data-mml-node="mi"><path data-c="1D44E" d="M498 144C498 153 493 158 482 158C474 158 468 151 465 137C445 58 421 18 394 18C377 18 368 32 368 60C368 73 372 97 381 132L438 357C443 376 445 387 445 392C445 412 434 422 412 422C391 422 377 410 370 387C349 424 319 442 281 442C216 442 159 409 109 343C63 281 40 217 40 150C40 63 91-11 175-11C218-11 260 12 300 58C311 20 345-11 392-11C461-11 482 70 498 144M341 374C350 353 355 339 355 330C355 326 354 321 353 314L304 122C301 111 294 99 285 87C248 41 212 18 177 18C138 18 118 48 118 107C118 131 124 167 136 215C157 300 187 357 224 388C244 405 263 413 282 413C309 413 329 400 341 374Z"/></g><g data-mml-node="merror" title="inner" transform="translate(529,0)"><rect data-background="true" width="1840" height="788" y="-205"/><title>inner</title><g data-mml-node="mrow"><g data-mml-node="mi"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g><g data-mml-node="mo" transform="translate(572,0)"><path data-c="2B" d="M698 274L413 274L413 559C413 575 405 583 389 583C373 583 365 575 365 559L365 274L80 274C64 274 56 266 56 250C56 234 64 226 80 226L365 226L365-59C365-75 373-83 389-83C405-83 413-75 413-59L413 226L698 226C714 226 722 234 722 250C722 263 711 274 698 274Z"/></g><g data-mml-node="mi" transform="translate(1350,0)"><path data-c="1D466" d="M163 442C118 442 83 417 58 367C39 328 29 301 29 286C29 277 34 272 45 272C59 272 60 278 64 293C87 372 119 412 160 412C173 412 180 403 180 385C180 369 175 346 164 317C126 214 107 145 107 108C107 32 154-13 231-13C264-13 295-1 323 23C288-109 233-175 158-175C124-175 101-164 89-142C129-140 149-121 149-85C149-60 136-47 109-47C72-47 50-78 50-115C50-170 100-205 158-205C273-205 367-99 392-1L486 377C489 388 490 396 490 401C490 421 479 431 458 431C442 431 429 423 420 408C414 387 409 369 406 354L342 97C332 61 279 16 234 16C196 16 177 41 177 92C177 134 194 198 227 284C240 319 247 343 247 357C247 406 212 442 163 442Z"/></g></g></g></g></g></g></g></svg></mjx-container>)FOLIAMERROR"},std::array<MerrorSourceBBox, 2>{{{2.369,0.583,0.205,0,0},{1.84,0.583,0.205,0,0}}},2},
  {mathml_scene_test::Case{"merror-authored-blue",mathml_scene_test::Metrics{16,8,640,false},merror5Nodes,merror5Attributes,R"FOLIAMERROR(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.464ex;" xmlns="http://www.w3.org/2000/svg" width="2.403ex" height="1.464ex" role="img" focusable="false" viewBox="0 -442 1062 647"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="merror"><rect data-background="true" width="1062" height="647" y="-205"/><g data-mml-node="mi" fill="blue" stroke="blue"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g><g data-mml-node="mi" fill="blue" stroke="blue" transform="translate(572,0)"><path data-c="1D466" d="M163 442C118 442 83 417 58 367C39 328 29 301 29 286C29 277 34 272 45 272C59 272 60 278 64 293C87 372 119 412 160 412C173 412 180 403 180 385C180 369 175 346 164 317C126 214 107 145 107 108C107 32 154-13 231-13C264-13 295-1 323 23C288-109 233-175 158-175C124-175 101-164 89-142C129-140 149-121 149-85C149-60 136-47 109-47C72-47 50-78 50-115C50-170 100-205 158-205C273-205 367-99 392-1L486 377C489 388 490 396 490 401C490 421 479 431 458 431C442 431 429 423 420 408C414 387 409 369 406 354L342 97C332 61 279 16 234 16C196 16 177 41 177 92C177 134 194 198 227 284C240 319 247 343 247 357C247 406 212 442 163 442Z"/></g></g></g></g></svg></mjx-container>)FOLIAMERROR"},std::array<MerrorSourceBBox, 2>{{{1.0619999999999998,0.442,0.205,0,0},{}}},1},
  {mathml_scene_test::Case{"merror-linked-nested-svg-child",mathml_scene_test::Metrics{16,8,640,false},merror6Nodes,merror6Attributes,R"FOLIAMERROR(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -5.09ex;" xmlns="http://www.w3.org/2000/svg" width="5.253ex" height="11.312ex" role="img" focusable="false" viewBox="0 -2750 2322 5000"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="merror"><rect data-background="true" width="2322" height="5000" y="-2250"/><a href="https://example.test/merror"><g data-mml-node="mrow"><rect data-hitbox="true" fill="none" stroke="none" pointer-events="all" width="2322" height="5000" x="0" y="-2250"/><g data-mml-node="mo"><path data-c="239B" d="M813 1450C820 1455 823 1462 823 1470C823 1487 815 1495 798 1495C793 1495 788 1493 784 1490C697 1423 614 1324 536 1192C399 956 277 603 277 253L277 0L385 0L385 253C385 407 403 562 438 719C500 999 630 1313 813 1450Z" transform="translate(0,1255)"/><path data-c="239D" d="M798 0C815 0 823 8 823 25C823 34 819 40 812 44C731 106 658 202 592 334C474 569 385 903 385 1242L385 1495L277 1495L277 1242C277 914 388 575 512 345C594 193 685 80 784 5C788 2 793 0 798 0Z" transform="translate(0,-2250)"/><svg width="875" height="2150" y="-825" x="0" viewBox="0 537.5 875 2150" xmlns="http://www.w3.org/2000/svg"><path data-c="239C" d="M277 0L385 0L385 498L277 498Z" transform="scale(1,6.476)"/></svg></g><g data-mml-node="mi" transform="translate(875,0)"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g><g data-mml-node="mo" transform="translate(1447,0)"><path data-c="239E" d="M490 0L598 0L598 253C598 581 487 920 363 1150C281 1301 190 1415 91 1490C87 1493 82 1495 77 1495C60 1495 52 1487 52 1470C52 1462 55 1455 62 1450C143 1389 216 1292 283 1160C402 924 490 593 490 253Z" transform="translate(0,1255)"/><path data-c="23A0" d="M598 1242L598 1495L490 1495L490 1242C490 1088 472 933 437 776C375 494 246 184 63 44C56 40 52 34 52 25C52 8 60 0 77 0C82 0 87 2 91 5C178 71 261 170 339 303C477 539 598 892 598 1242Z" transform="translate(0,-2250)"/><svg width="875" height="2150" y="-825" x="0" viewBox="0 537.5 875 2150" xmlns="http://www.w3.org/2000/svg"><path data-c="239F" d="M490 0L598 0L598 498L490 498Z" transform="scale(1,6.476)"/></svg></g></g></a></g></g></g></svg></mjx-container>)FOLIAMERROR"},std::array<MerrorSourceBBox, 2>{{{2.322,2.75,2.25,0,0},{}}},1}
};
