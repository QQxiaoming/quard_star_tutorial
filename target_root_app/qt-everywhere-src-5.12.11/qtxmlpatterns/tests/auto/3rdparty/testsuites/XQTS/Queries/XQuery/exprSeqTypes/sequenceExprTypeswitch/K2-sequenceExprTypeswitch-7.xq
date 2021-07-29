(:*******************************************************:)
(: Test: K2-sequenceExprTypeswitch-7                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A variable is only in scope for the case branch it is declared for(#3). :)
(:*******************************************************:)
typeswitch (1, 2, 3)
case node() return <e/>
case xs:integer* return $i
default $i return 1