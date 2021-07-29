(:*******************************************************:)
(: Test: K2-sequenceExprTypeswitch-5                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A variable is only in scope for the case branch it is declared for(#2). :)
(:*******************************************************:)
typeswitch (1, 2, 3)
case node() return $i
case $i as xs:integer return 1
default return true()