(:*******************************************************:)
(: Test: K-LetExprWithout-4                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A binding in a let-expression shadows global variables. :)
(:*******************************************************:)
declare variable $i := false();
declare variable $t := false();
deep-equal((let $i := true(), $t := true() return ($i, $t)),
(true(), true()))