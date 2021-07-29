(:*******************************************************:)
(: Test: K2-TokenizeFunc-7                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Tokenize on a single whitespace.             :)
(:*******************************************************:)
count(tokenize("a b", " ")),
count(tokenize("a b", "\s")),
string-join(tokenize("a b", " "), '|'),
string-join(tokenize("a b", "\s"), '|'),
tokenize("a b", " "),
tokenize("a b", "\s")