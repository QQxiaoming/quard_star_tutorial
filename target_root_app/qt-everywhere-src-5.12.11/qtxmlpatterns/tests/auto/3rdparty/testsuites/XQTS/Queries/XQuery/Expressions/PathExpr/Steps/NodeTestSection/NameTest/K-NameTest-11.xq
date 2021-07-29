(:*******************************************************:)
(: Test: K-NameTest-11                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Invalid string literals and nametests mixed, stressing tokenizer and parser code. :)
(:*******************************************************:)
normalize-unicode("f   oo") eq "f oo