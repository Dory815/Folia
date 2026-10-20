// Generated from pinned Source: tex-bussproofs-items-source.cjs --write-fixture
const std::array proofFrameTrees{
    ProofFrameTree{u"empty", u"12:inferredMrow0:0:1:0"},
    ProofFrameTree{u"one", u"2:mi1:x0:1:0"},
    ProofFrameTree{u"multiple", u"12:inferredMrow0:0:1:22:mi1:x0:1:02:mi1:y0:1:0"},
    ProofFrameTree{u"ignored-flags", u"12:inferredMrow0:0:1:22:mi1:x0:1:02:mi1:y0:1:0"},
    ProofFrameTree{u"ignored-force-single", u"2:mi1:x0:1:0"},
    ProofFrameTree{u"prefix", u"4:mrow0:0:1:22:mi1:p0:1:02:mi1:x0:1:0"},
    ProofFrameTree{u"grouped-prefix", u"4:mrow0:0:1:27:TeXAtom0:0:1:112:inferredMrow0:0:1:12:mi1:p0:1:02:mi1:x0:1:0"},
    ProofFrameTree{u"prefix-empty-proof", u"4:mrow0:0:1:12:mi1:p0:1:0"},
    ProofFrameTree{u"matching-end", u"2:mi1:x4:true1:0"},
    ProofFrameTree{u"matching-end-prefix", u"4:mrow0:4:true1:22:mi1:p0:1:02:mi1:x0:1:0"},
    ProofFrameTree{u"outer-replay", u"4:mrow0:4:true1:27:TeXAtom0:0:1:112:inferredMrow0:0:1:12:mi1:p0:1:02:mi1:x0:1:0"},
};
const std::array proofFrameErrors{
    ProofFrameError{u"stop", "EnvMissingEnd", u"Missing \\end{prooftree}"},
    ProofFrameError{u"stop-unnamed", "EnvMissingEnd", u"Missing \\end{}"},
    ProofFrameError{u"mismatched-end", "MissingBeginExtraEnd", u"Missing \\begin{other} or extra \\end{other}"},
    ProofFrameError{u"extra-close", "ExtraCloseMissingOpen", u"Extra close brace or missing open brace"},
    ProofFrameError{u"unfinished-group", "ExtraOpenMissingClose", u"Extra open brace or missing close brace"},
    ProofFrameError{u"unfinished-group-end", "ExtraOpenMissingClose", u"Extra open brace or missing close brace"},
};
