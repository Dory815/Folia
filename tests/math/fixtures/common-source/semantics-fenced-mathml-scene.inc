// Source MathJax 4.1.3 fb987178c2d279a99b0db5ea02e751691703955e
// Input: pinned direct MathML source for Common semantics/mfenced; not TeX input coverage.
// scope-sha256 9705d389538b2fc8bf782feea4372ace639b3727304f2971937bbcfb0042a0d1
// cases-sha256 4f04cfb63cd3feeab04c1da77567210464391921187ad559ccebdfb861ae1e3e
// profile-sha256 a5a03eed8a10872b9b435b9e602523a08a9f574e21cc5636e8377f0222811ac9
// input/mathml.js 8fc5382e4a81b8be02484c9f6de91618ce8a5c859af97b5175df6badf35035db | input/mathml.ts e027607ae93b8dedf7b8312b1b814c50cf8cd0c6b332f56edba02d1d3bd7be1b
// input/mathml/FindMathML.js fec27373d9b959de3511e5162eb98bb50d0b17a95e4e746a62f55fe625b86c2a | input/mathml/FindMathML.ts 1237e59c4a1b1e4852b4bf6986db1cbbb84efc0726426051021bcc6d2595dfd9
// input/mathml/MathMLCompile.js 0b190d56a673941b6b4d1c8e8632dfa894ed50ddccce484e9e8d142c6d1e8880 | input/mathml/MathMLCompile.ts 2bb1f7258daf68fa6d51ea585b08312c1bc407719bd6711651369db1d5b68349
constexpr std::array<mathml_scene_test::Node, 13> semantics0Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"semantics", false, 1, 0, 0, u""},
  {"mrow", false, 2, 0, 0, u""},
  {"mi", false, 3, 0, 0, u""},
  {"text", false, 4, 0, 0, u"x"},
  {"mo", false, 3, 0, 0, u""},
  {"text", false, 6, 0, 0, u"+"},
  {"mi", false, 3, 0, 0, u""},
  {"text", false, 8, 0, 0, u"y"},
  {"annotation", false, 2, 0, 1, u""},
  {"text", false, 10, 1, 0, u"SOURCE-HIDDEN-SEMANTICS-ONE"},
  {"annotation-xml", false, 2, 1, 1, u""}
}};
constexpr std::array<mathml_scene_test::Attribute, 2> semantics0Attributes{{
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/x-test-hidden", 0, false},
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/xml", 0, false}
}};

constexpr std::array<mathml_scene_test::Node, 14> semantics1Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"semantics", false, 1, 0, 0, u""},
  {"mstyle", false, 2, 0, 1, u""},
  {"inferredMrow", true, 3, 1, 0, u""},
  {"mrow", false, 4, 1, 0, u""},
  {"mi", false, 5, 1, 0, u""},
  {"text", false, 6, 1, 0, u"a"},
  {"mo", false, 5, 1, 0, u""},
  {"text", false, 8, 1, 0, u"+"},
  {"mi", false, 5, 1, 0, u""},
  {"text", false, 10, 1, 0, u"b"},
  {"annotation", false, 2, 1, 1, u""},
  {"text", false, 12, 2, 0, u"SOURCE-HIDDEN-SEMANTICS-TWO"}
}};
constexpr std::array<mathml_scene_test::Attribute, 2> semantics1Attributes{{
  {u"mathsize", mathml_scene_test::AttributeKind::text, u"150%", 0, false},
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/x-test-hidden", 0, false}
}};

constexpr std::array<mathml_scene_test::Node, 13> semantics2Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"mrow", false, 1, 0, 0, u""},
  {"mi", false, 2, 0, 0, u""},
  {"text", false, 3, 0, 0, u"x"},
  {"semantics", false, 2, 0, 0, u""},
  {"msub", false, 5, 0, 0, u""},
  {"mo", false, 6, 0, 2, u""},
  {"text", false, 7, 2, 0, u"+"},
  {"mi", false, 6, 2, 0, u""},
  {"text", false, 9, 2, 0, u"i"},
  {"annotation", false, 5, 2, 1, u""},
  {"text", false, 11, 3, 0, u"SOURCE-HIDDEN-SEMANTICS-THREE"}
}};
constexpr std::array<mathml_scene_test::Attribute, 3> semantics2Attributes{{
  {u"linebreak", mathml_scene_test::AttributeKind::text, u"newline", 0, false},
  {u"linebreakstyle", mathml_scene_test::AttributeKind::text, u"before", 0, false},
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/x-test-hidden", 0, false}
}};

constexpr std::array<mathml_scene_test::Node, 13> semantics3Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"mrow", false, 1, 0, 0, u""},
  {"semantics", false, 2, 0, 0, u""},
  {"msub", false, 3, 0, 0, u""},
  {"mo", false, 4, 0, 2, u""},
  {"text", false, 5, 2, 0, u"+"},
  {"mi", false, 4, 2, 0, u""},
  {"text", false, 7, 2, 0, u"i"},
  {"annotation", false, 3, 2, 1, u""},
  {"text", false, 9, 3, 0, u"SOURCE-HIDDEN-SEMANTICS-FOUR"},
  {"mi", false, 2, 3, 0, u""},
  {"text", false, 11, 3, 0, u"x"}
}};
constexpr std::array<mathml_scene_test::Attribute, 3> semantics3Attributes{{
  {u"linebreak", mathml_scene_test::AttributeKind::text, u"newline", 0, false},
  {u"linebreakstyle", mathml_scene_test::AttributeKind::text, u"after", 0, false},
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/x-test-hidden", 0, false}
}};

constexpr std::array<mathml_scene_test::Node, 7> mfenced4Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"mfenced", false, 1, 0, 0, u""},
  {"mi", false, 2, 0, 0, u""},
  {"text", false, 3, 0, 0, u"x"},
  {"mi", false, 2, 0, 0, u""},
  {"text", false, 5, 0, 0, u"y"}
}};
constexpr std::array<mathml_scene_test::Attribute, 0> mfenced4Attributes{{

}};

constexpr std::array<mathml_scene_test::Node, 11> mfenced5Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"mfenced", false, 1, 0, 3, u""},
  {"mi", false, 2, 3, 0, u""},
  {"text", false, 3, 3, 0, u"a"},
  {"mi", false, 2, 3, 0, u""},
  {"text", false, 5, 3, 0, u"b"},
  {"mi", false, 2, 3, 0, u""},
  {"text", false, 7, 3, 0, u"c"},
  {"mi", false, 2, 3, 0, u""},
  {"text", false, 9, 3, 0, u"d"}
}};
constexpr std::array<mathml_scene_test::Attribute, 3> mfenced5Attributes{{
  {u"open", mathml_scene_test::AttributeKind::text, u"[", 0, false},
  {u"close", mathml_scene_test::AttributeKind::text, u"]", 0, false},
  {u"separators", mathml_scene_test::AttributeKind::text, u";|", 0, false}
}};

constexpr std::array<mathml_scene_test::Node, 7> mfenced6Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"mfenced", false, 1, 0, 1, u""},
  {"mi", false, 2, 1, 0, u""},
  {"text", false, 3, 1, 0, u"x"},
  {"mi", false, 2, 1, 0, u""},
  {"text", false, 5, 1, 0, u"y"}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> mfenced6Attributes{{
  {u"separators", mathml_scene_test::AttributeKind::text, u"", 0, false}
}};

constexpr std::array<mathml_scene_test::Node, 5> mfenced7Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"mfenced", false, 1, 0, 3, u""},
  {"mi", false, 2, 3, 0, u""},
  {"text", false, 3, 3, 0, u"x"}
}};
constexpr std::array<mathml_scene_test::Attribute, 3> mfenced7Attributes{{
  {u"open", mathml_scene_test::AttributeKind::text, u"", 0, false},
  {u"close", mathml_scene_test::AttributeKind::text, u"", 0, false},
  {u"separators", mathml_scene_test::AttributeKind::text, u"", 0, false}
}};

constexpr std::array<mathml_scene_test::Node, 10> mfenced8Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"mfenced", false, 1, 0, 0, u""},
  {"mi", false, 2, 0, 0, u""},
  {"text", false, 3, 0, 0, u"x"},
  {"mfenced", false, 2, 0, 2, u""},
  {"mi", false, 5, 2, 0, u""},
  {"text", false, 6, 2, 0, u"y"},
  {"mi", false, 5, 2, 0, u""},
  {"text", false, 8, 2, 0, u"z"}
}};
constexpr std::array<mathml_scene_test::Attribute, 2> mfenced8Attributes{{
  {u"open", mathml_scene_test::AttributeKind::text, u"[", 0, false},
  {u"close", mathml_scene_test::AttributeKind::text, u"]", 0, false}
}};

constexpr std::array<mathml_scene_test::Node, 16> mfenced9Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"mfenced", false, 1, 0, 0, u""},
  {"mfrac", false, 2, 0, 0, u""},
  {"msup", false, 3, 0, 0, u""},
  {"mi", false, 4, 0, 0, u""},
  {"text", false, 5, 0, 0, u"x"},
  {"mn", false, 4, 0, 0, u""},
  {"text", false, 7, 0, 0, u"2"},
  {"msup", false, 3, 0, 0, u""},
  {"mi", false, 9, 0, 0, u""},
  {"text", false, 10, 0, 0, u"y"},
  {"mn", false, 9, 0, 0, u""},
  {"text", false, 12, 0, 0, u"3"},
  {"mi", false, 2, 0, 0, u""},
  {"text", false, 14, 0, 0, u"z"}
}};
constexpr std::array<mathml_scene_test::Attribute, 0> mfenced9Attributes{{

}};

constexpr std::array<mathml_scene_test::Node, 3> mfenced10Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u""},
  {"inferredMrow", true, 0, 0, 0, u""},
  {"mfenced", false, 1, 0, 0, u""}
}};
constexpr std::array<mathml_scene_test::Attribute, 0> mfenced10Attributes{{

}};

constexpr mathml_scene_test::Case sourceSemanticsCases[]{
  mathml_scene_test::Case{"semantics-presentation-hidden-annotations",mathml_scene_test::Metrics{16,8,640,false},semantics0Nodes,semantics0Attributes,R"FOLIAMATHML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.566ex;" xmlns="http://www.w3.org/2000/svg" width="1.294ex" height="2.262ex" role="img" focusable="false" viewBox="0 -750 572 1000"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="mrow"><g data-mml-node="mi"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g></g></g></g></g></svg><mjx-break size="3"> </mjx-break><svg style="vertical-align: -0.566ex;" xmlns="http://www.w3.org/2000/svg" width="3.372ex" height="2.262ex" role="img" focusable="false" viewBox="0 -750 1490.2 1000"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="mrow"><g data-mml-node="mo"><path data-c="2B" d="M698 274L413 274L413 559C413 575 405 583 389 583C373 583 365 575 365 559L365 274L80 274C64 274 56 266 56 250C56 234 64 226 80 226L365 226L365-59C365-75 373-83 389-83C405-83 413-75 413-59L413 226L698 226C714 226 722 234 722 250C722 263 711 274 698 274Z"/></g><g data-mml-node="mi" transform="translate(1000.2,0)"><path data-c="1D466" d="M163 442C118 442 83 417 58 367C39 328 29 301 29 286C29 277 34 272 45 272C59 272 60 278 64 293C87 372 119 412 160 412C173 412 180 403 180 385C180 369 175 346 164 317C126 214 107 145 107 108C107 32 154-13 231-13C264-13 295-1 323 23C288-109 233-175 158-175C124-175 101-164 89-142C129-140 149-121 149-85C149-60 136-47 109-47C72-47 50-78 50-115C50-170 100-205 158-205C273-205 367-99 392-1L486 377C489 388 490 396 490 401C490 421 479 431 458 431C442 431 429 423 420 408C414 387 409 369 406 354L342 97C332 61 279 16 234 16C196 16 177 41 177 92C177 134 194 198 227 284C240 319 247 343 247 357C247 406 212 442 163 442Z"/></g></g></g></g></g></svg></mjx-container>)FOLIAMATHML"},
  mathml_scene_test::Case{"semantics-scaled-nested-presentation",mathml_scene_test::Metrics{16,8,640,false},semantics1Nodes,semantics1Attributes,R"FOLIAMATHML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.188ex;" xmlns="http://www.w3.org/2000/svg" width="4.933ex" height="1.758ex" role="img" focusable="false" viewBox="0 -694 2180.4 777"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="mstyle" transform="scale(1.5)"><g data-mml-node="mrow"><g data-mml-node="mi"><path data-c="1D44E" d="M498 144C498 153 493 158 482 158C474 158 468 151 465 137C445 58 421 18 394 18C377 18 368 32 368 60C368 73 372 97 381 132L438 357C443 376 445 387 445 392C445 412 434 422 412 422C391 422 377 410 370 387C349 424 319 442 281 442C216 442 159 409 109 343C63 281 40 217 40 150C40 63 91-11 175-11C218-11 260 12 300 58C311 20 345-11 392-11C461-11 482 70 498 144M341 374C350 353 355 339 355 330C355 326 354 321 353 314L304 122C301 111 294 99 285 87C248 41 212 18 177 18C138 18 118 48 118 107C118 131 124 167 136 215C157 300 187 357 224 388C244 405 263 413 282 413C309 413 329 400 341 374Z"/></g><g data-mml-node="mo" transform="translate(751.2,0)"><path data-c="2B" d="M698 274L413 274L413 559C413 575 405 583 389 583C373 583 365 575 365 559L365 274L80 274C64 274 56 266 56 250C56 234 64 226 80 226L365 226L365-59C365-75 373-83 389-83C405-83 413-75 413-59L413 226L698 226C714 226 722 234 722 250C722 263 711 274 698 274Z"/></g><g data-mml-node="mi" transform="translate(1751.4,0)"><path data-c="1D44F" d="M281 445C245 445 209 428 174 394L243 679C241 688 238 694 226 694C193 694 120 685 107 684C92 682 84 675 84 660C84 650 93 645 112 645C131 645 157 646 157 632C157 628 152 608 143 571L63 249C52 207 47 173 47 148C47 62 93-11 175-11C240-11 297 22 347 89C392 151 415 216 415 283C415 370 364 445 281 445M279 415C318 415 337 385 337 326C337 299 331 263 319 218C297 133 268 75 233 44C213 27 194 19 175 19C134 19 114 51 114 115C114 137 119 170 129 214L151 304C154 319 159 330 165 338C204 389 242 415 279 415Z"/></g></g></g></g></g></g></svg></mjx-container>)FOLIAMATHML"},
  mathml_scene_test::Case{"semantics-forced-break-before-embellished-operator",mathml_scene_test::Metrics{16,8,72,false},semantics2Nodes,semantics2Attributes,R"FOLIAMATHML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -3.281ex;" xmlns="http://www.w3.org/2000/svg" width="2.5ex" height="4.977ex" role="img" focusable="false" viewBox="0 -750 1105 2200"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mjx-linestack="true"><g data-mjx-linebox="true" data-mjx-lineno="0"><g data-mml-node="mrow"><g data-mml-node="mi"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g><g data-mml-node="semantics" transform="translate(572,0)"/></g></g><g data-mjx-linebox="true" data-mjx-lineno="1" transform="translate(0,-1000)"><g data-mml-node="mrow"><g data-mml-node="semantics"><g data-mml-node="msub"><g data-mml-node="mo"><path data-c="2B" d="M698 274L413 274L413 559C413 575 405 583 389 583C373 583 365 575 365 559L365 274L80 274C64 274 56 266 56 250C56 234 64 226 80 226L365 226L365-59C365-75 373-83 389-83C405-83 413-75 413-59L413 226L698 226C714 226 722 234 722 250C722 263 711 274 698 274Z"/></g><g data-mml-node="mi" transform="translate(811,-150) scale(0.707)"><path data-c="1D456" d="M284 621C284 648 271 661 244 661C216 661 188 633 188 605C188 578 202 565 229 565C257 565 284 593 284 621M259 138C237 59 205 19 164 19C151 19 144 28 144 47C144 64 173 150 232 306C240 329 244 347 244 360C244 409 210 445 161 445C118 445 84 420 59 369C39 328 29 301 29 288C29 279 34 275 45 275C58 275 60 281 64 295C87 375 118 415 158 415C171 415 178 406 178 387C178 373 175 357 168 338C145 275 121 208 101 155C86 115 78 88 78 74C78 25 114-11 162-11C205-11 239 14 264 64C283 103 293 130 293 145C293 154 288 159 277 159C274 159 259 150 259 138Z"/></g></g></g></g></g></g></g></g></svg></mjx-container>)FOLIAMATHML"},
  mathml_scene_test::Case{"semantics-forced-break-after-embellished-operator",mathml_scene_test::Metrics{16,8,72,false},semantics3Nodes,semantics3Attributes,R"FOLIAMATHML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -3.281ex;" xmlns="http://www.w3.org/2000/svg" width="2.5ex" height="4.977ex" role="img" focusable="false" viewBox="0 -750 1105 2200"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mjx-linestack="true"><g data-mjx-linebox="true" data-mjx-lineno="0"><g data-mml-node="mrow"><g data-mml-node="semantics"><g data-mml-node="msub"><g data-mml-node="mo"><path data-c="2B" d="M698 274L413 274L413 559C413 575 405 583 389 583C373 583 365 575 365 559L365 274L80 274C64 274 56 266 56 250C56 234 64 226 80 226L365 226L365-59C365-75 373-83 389-83C405-83 413-75 413-59L413 226L698 226C714 226 722 234 722 250C722 263 711 274 698 274Z"/></g><g data-mml-node="mi" transform="translate(811,-150) scale(0.707)"><path data-c="1D456" d="M284 621C284 648 271 661 244 661C216 661 188 633 188 605C188 578 202 565 229 565C257 565 284 593 284 621M259 138C237 59 205 19 164 19C151 19 144 28 144 47C144 64 173 150 232 306C240 329 244 347 244 360C244 409 210 445 161 445C118 445 84 420 59 369C39 328 29 301 29 288C29 279 34 275 45 275C58 275 60 281 64 295C87 375 118 415 158 415C171 415 178 406 178 387C178 373 175 357 168 338C145 275 121 208 101 155C86 115 78 88 78 74C78 25 114-11 162-11C205-11 239 14 264 64C283 103 293 130 293 145C293 154 288 159 277 159C274 159 259 150 259 138Z"/></g></g></g></g></g><g data-mjx-linebox="true" data-mjx-lineno="1" transform="translate(0,-1000)"><g data-mml-node="mrow"><g data-mml-node="semantics"/><g data-mml-node="mi"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g></g></g></g></g></g></svg></mjx-container>)FOLIAMATHML"}
};

constexpr mathml_scene_test::Case sourceMfencedCases[]{
  mathml_scene_test::Case{"mfenced-default-two-children",mathml_scene_test::Metrics{16,8,640,false},mfenced4Nodes,mfenced4Attributes,R"FOLIAMATHML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.561ex;" xmlns="http://www.w3.org/2000/svg" width="5.169ex" height="2.253ex" role="img" focusable="false" viewBox="0 -748 2284.7 996"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="mfenced"><g data-mml-node="mo"><path data-c="28" d="M318-248C327-248 332-243 332-234C332-231 330-227 327-223C275-183 233-117 202-26C175 53 161 131 161 208L161 292C161 369 175 447 202 526C233 617 275 683 327 723C330 726 332 730 332 734C332 743 327 748 318 748C317 748 314 747 311 745C251 699 201 631 160 540C121 453 101 371 101 292L101 208C101 129 121 47 160-40C201-131 251-199 311-245C314-247 317-248 318-248Z"/></g><g data-mml-node="mi" transform="translate(389,0)"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g><g data-mml-node="mo" transform="translate(961,0)"><path data-c="2C" d="M139 106C107 106 86 82 86 50C86 20 109-5 139-5C153-5 165-1 174 8L175 0C175-63 154-117 112-160C105-168 101-174 101-178C101-188 105-193 114-193C123-193 135-181 152-158C186-110 203-57 203 0C203 53 185 106 139 106Z"/></g><g data-mml-node="mi" transform="translate(1405.7,0)"><path data-c="1D466" d="M163 442C118 442 83 417 58 367C39 328 29 301 29 286C29 277 34 272 45 272C59 272 60 278 64 293C87 372 119 412 160 412C173 412 180 403 180 385C180 369 175 346 164 317C126 214 107 145 107 108C107 32 154-13 231-13C264-13 295-1 323 23C288-109 233-175 158-175C124-175 101-164 89-142C129-140 149-121 149-85C149-60 136-47 109-47C72-47 50-78 50-115C50-170 100-205 158-205C273-205 367-99 392-1L486 377C489 388 490 396 490 401C490 421 479 431 458 431C442 431 429 423 420 408C414 387 409 369 406 354L342 97C332 61 279 16 234 16C196 16 177 41 177 92C177 134 194 198 227 284C240 319 247 343 247 357C247 406 212 442 163 442Z"/></g><g data-mml-node="mo" transform="translate(1895.7,0)"><path data-c="29" d="M78-245C138-199 188-131 229-40C268 47 288 129 288 208L288 292C288 371 268 453 229 540C188 631 138 699 78 745C75 747 72 748 71 748C62 748 57 743 57 734C57 730 59 726 62 723C114 683 156 617 187 526C214 447 228 369 228 292L228 208C228 131 214 53 187-26C156-117 114-183 62-223C59-227 57-231 57-234C57-243 62-248 71-248C72-248 75-247 78-245Z"/></g></g></g></g></svg></mjx-container>)FOLIAMATHML"},
  mathml_scene_test::Case{"mfenced-custom-repeated-separator",mathml_scene_test::Metrics{16,8,640,false},mfenced5Nodes,mfenced5Attributes,R"FOLIAMATHML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.566ex;" xmlns="http://www.w3.org/2000/svg" width="7.845ex" height="2.262ex" role="img" focusable="false" viewBox="0 -750 3467.7 1000"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="mfenced"><g data-mml-node="mo"><path data-c="5B" d="M233-202L159-202L159 702L233 702C248 702 256 710 256 726C256 742 248 750 233 750L114 750L114-250L233-250C248-250 256-242 256-226C256-214 245-202 233-202Z"/></g><g data-mml-node="mi" transform="translate(278,0)"><path data-c="1D44E" d="M498 144C498 153 493 158 482 158C474 158 468 151 465 137C445 58 421 18 394 18C377 18 368 32 368 60C368 73 372 97 381 132L438 357C443 376 445 387 445 392C445 412 434 422 412 422C391 422 377 410 370 387C349 424 319 442 281 442C216 442 159 409 109 343C63 281 40 217 40 150C40 63 91-11 175-11C218-11 260 12 300 58C311 20 345-11 392-11C461-11 482 70 498 144M341 374C350 353 355 339 355 330C355 326 354 321 353 314L304 122C301 111 294 99 285 87C248 41 212 18 177 18C138 18 118 48 118 107C118 131 124 167 136 215C157 300 187 357 224 388C244 405 263 413 282 413C309 413 329 400 341 374Z"/></g><g data-mml-node="mo" transform="translate(807,0)"><path data-c="3B" d="M192 375C192 405 169 431 139 431C109 431 86 405 86 375C86 344 109 318 139 318C169 318 192 345 192 375M86 54C86 22 107-2 139-2C150-2 160 1 167 5C166-61 146-117 108-163C103-169 101-174 101-178C101-188 105-193 114-193C119-193 128-185 141-168C177-121 195-64 195 4C195 75 176 110 139 110C107 110 86 86 86 54Z"/></g><g data-mml-node="mi" transform="translate(1251.7,0)"><path data-c="1D44F" d="M281 445C245 445 209 428 174 394L243 679C241 688 238 694 226 694C193 694 120 685 107 684C92 682 84 675 84 660C84 650 93 645 112 645C131 645 157 646 157 632C157 628 152 608 143 571L63 249C52 207 47 173 47 148C47 62 93-11 175-11C240-11 297 22 347 89C392 151 415 216 415 283C415 370 364 445 281 445M279 415C318 415 337 385 337 326C337 299 331 263 319 218C297 133 268 75 233 44C213 27 194 19 175 19C134 19 114 51 114 115C114 137 119 170 129 214L151 304C154 319 159 330 165 338C204 389 242 415 279 415Z"/></g><g data-mml-node="mo" transform="translate(1680.7,0)"><path data-c="7C" d="M139-250C152-250 159-242 159-226L159 726C159 742 152 750 139 750C126 750 119 742 119 726L119-226C119-242 126-250 139-250Z"/></g><g data-mml-node="mi" transform="translate(1958.7,0)"><path data-c="1D450" d="M328 325C328 300 341 287 368 287C404 287 427 318 427 354C427 410 368 442 308 442C239 442 176 412 122 353C68 294 41 229 41 159C41 61 106-11 204-11C257-11 304 2 345 27C379 48 404 69 420 90C427 99 430 106 430 109C430 120 425 126 414 126C409 126 404 122 398 114C365 70 324 42 277 30C246 22 223 18 206 18C149 18 121 53 121 122C121 184 150 274 174 317C198 361 249 413 308 413C346 413 372 402 386 381C355 378 328 358 328 325Z"/></g><g data-mml-node="mo" transform="translate(2391.7,0)"><path data-c="7C" d="M139-250C152-250 159-242 159-226L159 726C159 742 152 750 139 750C126 750 119 742 119 726L119-226C119-242 126-250 139-250Z"/></g><g data-mml-node="mi" transform="translate(2669.7,0)"><path data-c="1D451" d="M429 632L370 389C349 426 320 445 281 445C216 445 159 412 109 345C63 283 40 218 40 151C40 63 91-11 175-11C218-11 260 12 300 59C311 21 345-11 392-11C461-11 483 71 498 145C498 154 493 159 483 159C474 159 468 152 465 138C445 59 421 19 394 19C377 19 368 33 368 60C368 75 370 91 374 107L516 679L516 683C513 690 507 694 499 694C4)FOLIAMATHML"
      R"FOLIAMATHML(76 694 434 690 374 683C363 682 357 674 357 660C357 650 366 645 384 645C403 645 429 646 429 632M341 376C351 355 356 341 356 332C355 328 354 323 353 316L304 122C301 111 294 99 285 87C248 42 212 19 177 19C138 19 118 49 118 108C118 132 124 168 136 216C157 301 187 359 224 390C244 407 263 415 282 415C309 415 329 402 341 376Z"/></g><g data-mml-node="mo" transform="translate(3189.7,0)"><path data-c="5D" d="M45-250L164-250L164 750L45 750C30 750 22 742 22 726C22 710 30 702 45 702L119 702L119-202L45-202C30-202 22-210 22-226C22-242 30-250 45-250Z"/></g></g></g></g></svg></mjx-container>)FOLIAMATHML"},
  mathml_scene_test::Case{"mfenced-empty-separator",mathml_scene_test::Metrics{16,8,640,false},mfenced6Nodes,mfenced6Attributes,R"FOLIAMATHML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.561ex;" xmlns="http://www.w3.org/2000/svg" width="4.163ex" height="2.253ex" role="img" focusable="false" viewBox="0 -748 1840 996"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="mfenced"><g data-mml-node="mo"><path data-c="28" d="M318-248C327-248 332-243 332-234C332-231 330-227 327-223C275-183 233-117 202-26C175 53 161 131 161 208L161 292C161 369 175 447 202 526C233 617 275 683 327 723C330 726 332 730 332 734C332 743 327 748 318 748C317 748 314 747 311 745C251 699 201 631 160 540C121 453 101 371 101 292L101 208C101 129 121 47 160-40C201-131 251-199 311-245C314-247 317-248 318-248Z"/></g><g data-mml-node="mi" transform="translate(389,0)"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g><g data-mml-node="mi" transform="translate(961,0)"><path data-c="1D466" d="M163 442C118 442 83 417 58 367C39 328 29 301 29 286C29 277 34 272 45 272C59 272 60 278 64 293C87 372 119 412 160 412C173 412 180 403 180 385C180 369 175 346 164 317C126 214 107 145 107 108C107 32 154-13 231-13C264-13 295-1 323 23C288-109 233-175 158-175C124-175 101-164 89-142C129-140 149-121 149-85C149-60 136-47 109-47C72-47 50-78 50-115C50-170 100-205 158-205C273-205 367-99 392-1L486 377C489 388 490 396 490 401C490 421 479 431 458 431C442 431 429 423 420 408C414 387 409 369 406 354L342 97C332 61 279 16 234 16C196 16 177 41 177 92C177 134 194 198 227 284C240 319 247 343 247 357C247 406 212 442 163 442Z"/></g><g data-mml-node="mo" transform="translate(1451,0)"><path data-c="29" d="M78-245C138-199 188-131 229-40C268 47 288 129 288 208L288 292C288 371 268 453 229 540C188 631 138 699 78 745C75 747 72 748 71 748C62 748 57 743 57 734C57 730 59 726 62 723C114 683 156 617 187 526C214 447 228 369 228 292L228 208C228 131 214 53 187-26C156-117 114-183 62-223C59-227 57-231 57-234C57-243 62-248 71-248C72-248 75-247 78-245Z"/></g></g></g></g></svg></mjx-container>)FOLIAMATHML"},
  mathml_scene_test::Case{"mfenced-absent-open-close",mathml_scene_test::Metrics{16,8,640,false},mfenced7Nodes,mfenced7Attributes,R"FOLIAMATHML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.025ex;" xmlns="http://www.w3.org/2000/svg" width="1.294ex" height="1.025ex" role="img" focusable="false" viewBox="0 -442 572 453"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="mfenced"><g data-mml-node="mi"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g></g></g></g></svg></mjx-container>)FOLIAMATHML"},
  mathml_scene_test::Case{"mfenced-nested",mathml_scene_test::Metrics{16,8,640,false},mfenced8Nodes,mfenced8Attributes,R"FOLIAMATHML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.566ex;" xmlns="http://www.w3.org/2000/svg" width="8.867ex" height="2.262ex" role="img" focusable="false" viewBox="0 -750 3919 1000"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="mfenced"><g data-mml-node="mo"><path data-c="28" d="M318-248C327-248 332-243 332-234C332-231 330-227 327-223C275-183 233-117 202-26C175 53 161 131 161 208L161 292C161 369 175 447 202 526C233 617 275 683 327 723C330 726 332 730 332 734C332 743 327 748 318 748C317 748 314 747 311 745C251 699 201 631 160 540C121 453 101 371 101 292L101 208C101 129 121 47 160-40C201-131 251-199 311-245C314-247 317-248 318-248Z"/></g><g data-mml-node="mi" transform="translate(389,0)"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g><g data-mml-node="mo" transform="translate(961,0)"><path data-c="2C" d="M139 106C107 106 86 82 86 50C86 20 109-5 139-5C153-5 165-1 174 8L175 0C175-63 154-117 112-160C105-168 101-174 101-178C101-188 105-193 114-193C123-193 135-181 152-158C186-110 203-57 203 0C203 53 185 106 139 106Z"/></g><g data-mml-node="mfenced" transform="translate(1405.7,0)"><g data-mml-node="mo" transform="translate(166.7,0)"><path data-c="5B" d="M233-202L159-202L159 702L233 702C248 702 256 710 256 726C256 742 248 750 233 750L114 750L114-250L233-250C248-250 256-242 256-226C256-214 245-202 233-202Z"/></g><g data-mml-node="mi" transform="translate(444.7,0)"><path data-c="1D466" d="M163 442C118 442 83 417 58 367C39 328 29 301 29 286C29 277 34 272 45 272C59 272 60 278 64 293C87 372 119 412 160 412C173 412 180 403 180 385C180 369 175 346 164 317C126 214 107 145 107 108C107 32 154-13 231-13C264-13 295-1 323 23C288-109 233-175 158-175C124-175 101-164 89-142C129-140 149-121 149-85C149-60 136-47 109-47C72-47 50-78 50-115C50-170 100-205 158-205C273-205 367-99 392-1L486 377C489 388 490 396 490 401C490 421 479 431 458 431C442 431 429 423 420 408C414 387 409 369 406 354L342 97C332 61 279 16 234 16C196 16 177 41 177 92C177 134 194 198 227 284C240 319 247 343 247 357C247 406 212 442 163 442Z"/></g><g data-mml-node="mo" transform="translate(934.7,0)"><path data-c="2C" d="M139 106C107 106 86 82 86 50C86 20 109-5 139-5C153-5 165-1 174 8L175 0C175-63 154-117 112-160C105-168 101-174 101-178C101-188 105-193 114-193C123-193 135-181 152-158C186-110 203-57 203 0C203 53 185 106 139 106Z"/></g><g data-mml-node="mi" transform="translate(1379.3,0)"><path data-c="1D467" d="M162 92L145 90C193 137 230 171 255 194L351 283C374 304 398 330 421 359C452 397 467 420 467 428C467 437 462 442 452 442C445 442 439 438 434 429C409 388 386 368 364 368C342 368 326 386 317 398C293 427 269 442 246 442C220 442 193 430 165 404C137 378 123 352 123 326C123 316 128 311 139 311C146 311 152 316 156 325C167 3)FOLIAMATHML"
      R"FOLIAMATHML(54 194 368 235 368C249 368 269 363 296 354C320 345 343 340 364 339C308 284 265 244 234 217L145 134C130 119 111 99 90 74C59 35 43 12 43 3C43-6 48-11 59-11C65-11 71-6 78 4C103 43 130 63 158 63C174 63 192 51 212 26C232 1 255-11 278-11C312-11 346 7 382 42C418 77 436 111 436 144C436 153 431 158 420 158C413 158 407 153 402 143C387 100 344 63 288 63C264 63 186 92 162 92Z"/></g><g data-mml-node="mo" transform="translate(1846.3,0)"><path data-c="5D" d="M45-250L164-250L164 750L45 750C30 750 22 742 22 726C22 710 30 702 45 702L119 702L119-202L45-202C30-202 22-210 22-226C22-242 30-250 45-250Z"/></g></g><g data-mml-node="mo" transform="translate(3530,0)"><path data-c="29" d="M78-245C138-199 188-131 229-40C268 47 288 129 288 208L288 292C288 371 268 453 229 540C188 631 138 699 78 745C75 747 72 748 71 748C62 748 57 743 57 734C57 730 59 726 62 723C114 683 156 617 187 526C214 447 228 369 228 292L228 208C228 131 214 53 187-26C156-117 114-183 62-223C59-227 57-231 57-234C57-243 62-248 71-248C72-248 75-247 78-245Z"/></g></g></g></g></svg></mjx-container>)FOLIAMATHML"},
  mathml_scene_test::Case{"mfenced-tall-fraction-content",mathml_scene_test::Metrics{16,8,640,false},mfenced9Nodes,mfenced9Attributes,R"FOLIAMATHML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -1.182ex;" xmlns="http://www.w3.org/2000/svg" width="7.038ex" height="3.407ex" role="img" focusable="false" viewBox="0 -983.7 3110.8 1506"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="mfenced"><g data-mml-node="mo"><path data-c="28" d="M444-472C455-472 461-466 461-455C461-449 459-445 454-442C382-387 325-294 282-164C245-53 226 57 226 164L226 336C226 443 245 552 282 664C325 793 382 886 454 942C459 945 461 949 461 955C461 966 455 972 444 972C440 972 437 971 434 968C353 905 285 809 232 681C181 560 156 445 156 336L156 164C156 55 181-60 232-181C285-310 353-405 434-468C437-471 440-472 444-472Z"/></g><g data-mml-node="mfrac" transform="translate(523,0)"><g data-mml-node="msup" transform="translate(220,394) scale(0.707)"><g data-mml-node="mi"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g><g data-mml-node="mn" transform="translate(605,363) scale(0.707)"><path data-c="32" d="M237 666C186 666 143 648 106 612C69 576 50 534 50 483C50 449 75 424 106 424C136 424 161 450 161 480C161 513 137 536 105 536C102 536 100 536 98 535C117 584 161 627 224 627C306 627 352 556 352 470C352 403 318 331 250 255L62 43C49 28 50 29 50 0L421 0L450 180L417 180C409 129 402 100 396 91C391 86 361 84 306 84L139 84L236 179C304 243 390 312 419 365C439 400 449 435 449 470C449 588 357 666 237 666Z"/></g></g><g data-mml-node="msup" transform="translate(249,-377.4) scale(0.707)"><g data-mml-node="mi"><path data-c="1D466" d="M163 442C118 442 83 417 58 367C39 328 29 301 29 286C29 277 34 272 45 272C59 272 60 278 64 293C87 372 119 412 160 412C173 412 180 403 180 385C180 369 175 346 164 317C126 214 107 145 107 108C107 32 154-13 231-13C264-13 295-1 323 23C288-109 233-175 158-175C124-175 101-164 89-142C129-140 149-121 149-85C149-60 136-47 109-47C72-47 50-78 50-115C50-170 100-205 158-205C273-205 367-99 392-1L486 377C489 388 490 396 490 401C490 421 479 431 458 431C442 431 429 423 420 408C414 387 409 369 406 354L342 97C332 61 279 16 234 16C196 16 177 41 177 92C177 134 194 198 227 284C240 319 247 343 247 357C247 406 212 442 163 442Z"/></g><g data-mml-node="mn" transform="translate(523,289) scale(0.707)"><path data-c="33" d="M303 353C369 378 431 441 431 526C431 569 410 604 369 631C333 654 292 666 246 666C201 666 162 654 127 631C88 605 68 571 68 528C68 495 90 472 122 472C154 472 176 495 176 527C176 560 157 578 119 580C145 615 186 633 242 633C302 633 332 598 332 527C332 485 324 450 309 421C282 373 245 364 183 364C171 362 165 357 165 348C165 333 172 333 192 333L235 333C310 333 348 280 348 173C348 88 317 14 241 14C176 14 128 36 99 80C134 79 160 105 160 139C160 173 135 198 101 198C62 198 42 178 42 137C42 88 64 49 108 18C147-9 193-22 244-22C301-22 350-3 393 34C436 71 457)FOLIAMATHML"
      R"FOLIAMATHML( 117 457 173C457 267 383 332 303 353Z"/></g></g><rect width="913.2" height="60" x="120" y="220"/></g><g data-mml-node="mo" transform="translate(1676.2,0)"><path data-c="2C" d="M139 106C107 106 86 82 86 50C86 20 109-5 139-5C153-5 165-1 174 8L175 0C175-63 154-117 112-160C105-168 101-174 101-178C101-188 105-193 114-193C123-193 135-181 152-158C186-110 203-57 203 0C203 53 185 106 139 106Z"/></g><g data-mml-node="mi" transform="translate(2120.8,0)"><path data-c="1D467" d="M162 92L145 90C193 137 230 171 255 194L351 283C374 304 398 330 421 359C452 397 467 420 467 428C467 437 462 442 452 442C445 442 439 438 434 429C409 388 386 368 364 368C342 368 326 386 317 398C293 427 269 442 246 442C220 442 193 430 165 404C137 378 123 352 123 326C123 316 128 311 139 311C146 311 152 316 156 325C167 354 194 368 235 368C249 368 269 363 296 354C320 345 343 340 364 339C308 284 265 244 234 217L145 134C130 119 111 99 90 74C59 35 43 12 43 3C43-6 48-11 59-11C65-11 71-6 78 4C103 43 130 63 158 63C174 63 192 51 212 26C232 1 255-11 278-11C312-11 346 7 382 42C418 77 436 111 436 144C436 153 431 158 420 158C413 158 407 153 402 143C387 100 344 63 288 63C264 63 186 92 162 92Z"/></g><g data-mml-node="mo" transform="translate(2587.8,0)"><path data-c="29" d="M89-468C170-405 238-310 291-181C342-60 367 55 367 164L367 336C367 445 342 560 291 681C238 810 170 905 89 968C85 971 82 972 79 972C68 972 62 966 62 955C62 950 64 945 68 942C140 887 198 795 241 664C278 552 297 443 297 336L297 164C297 57 278-52 241-164C198-295 140-387 68-442C64-445 62-450 62-455C62-466 68-472 79-472C82-472 86-471 89-468Z"/></g></g></g></g></svg></mjx-container>)FOLIAMATHML"},
  mathml_scene_test::Case{"mfenced-empty-children",mathml_scene_test::Metrics{16,8,640,false},mfenced10Nodes,mfenced10Attributes,R"FOLIAMATHML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.561ex;" xmlns="http://www.w3.org/2000/svg" width="1.76ex" height="2.253ex" role="img" focusable="false" viewBox="0 -748 778 996"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="mfenced"><g data-mml-node="mo"><path data-c="28" d="M318-248C327-248 332-243 332-234C332-231 330-227 327-223C275-183 233-117 202-26C175 53 161 131 161 208L161 292C161 369 175 447 202 526C233 617 275 683 327 723C330 726 332 730 332 734C332 743 327 748 318 748C317 748 314 747 311 745C251 699 201 631 160 540C121 453 101 371 101 292L101 208C101 129 121 47 160-40C201-131 251-199 311-245C314-247 317-248 318-248Z"/></g><g data-mml-node="mo" transform="translate(389,0)"><path data-c="29" d="M78-245C138-199 188-131 229-40C268 47 288 129 288 208L288 292C288 371 268 453 229 540C188 631 138 699 78 745C75 747 72 748 71 748C62 748 57 743 57 734C57 730 59 726 62 723C114 683 156 617 187 526C214 447 228 369 228 292L228 208C228 131 214 53 187-26C156-117 114-183 62-223C59-227 57-231 57-234C57-243 62-248 71-248C72-248 75-247 78-245Z"/></g></g></g></g></svg></mjx-container>)FOLIAMATHML"}
};
