(:*******************************************************:)
(: Test: K2-sequenceExprTypeswitch-2                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A default clause must be specified.          :)
(:*******************************************************:)
typeswitch(current-time())
case node() return 0
case xs:integer return 3
case xs:anyAtomicType return true()