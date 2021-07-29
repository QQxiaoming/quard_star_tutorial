(:*******************************************************:)
(: Test: K2-sequenceExprTypeswitch-11                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A variable declared in the default branch doesn't carry over to a subsequent typeswitch. :)
(:*******************************************************:)
typeswitch (1, 2, 3)
case node() return <e/>
default $i return 1

,

typeswitch (1, 2, 3)
case xs:integer* return $i 
default return 1