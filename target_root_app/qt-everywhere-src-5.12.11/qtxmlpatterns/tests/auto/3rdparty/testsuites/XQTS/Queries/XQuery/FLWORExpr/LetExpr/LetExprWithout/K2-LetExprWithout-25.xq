(:*******************************************************:)
(: Test: K2-LetExprWithout-25                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A combination of expressions that triggers a bug in some parsers. :)
(:*******************************************************:)
let $a := 1 return
for $b in 1
return
if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else if ($b) then 1
else ()