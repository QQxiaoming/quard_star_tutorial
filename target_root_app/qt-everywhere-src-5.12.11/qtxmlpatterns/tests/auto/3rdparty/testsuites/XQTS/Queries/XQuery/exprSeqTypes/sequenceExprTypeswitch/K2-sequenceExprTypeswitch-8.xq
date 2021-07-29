(:*******************************************************:)
(: Test: K2-sequenceExprTypeswitch-8                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A type declaration is not allowed in the default branch. :)
(:*******************************************************:)
typeswitch (1, 2, 3)
case node() return <e/>
case xs:integer return 3
default $i as item() return 1