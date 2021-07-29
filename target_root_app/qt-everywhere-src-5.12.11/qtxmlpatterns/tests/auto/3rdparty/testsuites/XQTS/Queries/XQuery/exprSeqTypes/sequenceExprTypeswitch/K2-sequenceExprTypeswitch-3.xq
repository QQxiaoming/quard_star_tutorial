(:*******************************************************:)
(: Test: K2-sequenceExprTypeswitch-3                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Parenteses must be specified for the expression that's switched. :)
(:*******************************************************:)
typeswitch 1
case node() return 0
case xs:integer return 3
default return true()