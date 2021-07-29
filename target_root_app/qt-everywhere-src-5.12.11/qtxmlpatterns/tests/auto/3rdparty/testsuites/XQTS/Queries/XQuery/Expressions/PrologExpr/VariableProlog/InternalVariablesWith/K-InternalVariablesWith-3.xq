(:*******************************************************:)
(: Test: K-InternalVariablesWith-3                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A variable declaration whose source expression doesn't match the declared type, and where it typically is difficult to deduce statically. :)
(:*******************************************************:)
declare variable $myVar as xs:integer := subsequence((1, 2, "a string"), 3, 1); $myVar eq 3