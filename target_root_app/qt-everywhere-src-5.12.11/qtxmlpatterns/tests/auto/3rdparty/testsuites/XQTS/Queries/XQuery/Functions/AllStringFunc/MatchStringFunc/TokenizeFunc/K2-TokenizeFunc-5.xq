(:*******************************************************:)
(: Test: K2-TokenizeFunc-5                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Tokenize a sequence of words.                :)
(:*******************************************************:)
deep-equal(fn:tokenize("The cat     sat on the mat", "\s+"),
("The", "cat", "sat", "on", "the", "mat")),
count(fn:tokenize("The      cat sat on the       mat", "\s+")),
count(fn:tokenize("         The      cat sat on the       mat           ", "\s+")),
fn:tokenize("The cat sat       on the       mat", "\s+")