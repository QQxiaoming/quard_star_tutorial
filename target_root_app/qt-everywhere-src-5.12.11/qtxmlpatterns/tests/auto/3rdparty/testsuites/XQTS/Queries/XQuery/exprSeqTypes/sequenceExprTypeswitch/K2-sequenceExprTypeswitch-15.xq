(:*******************************************************:)
(: Test: K2-sequenceExprTypeswitch-15                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use the focus from within a typeswitch's case-branch. :)
(:*******************************************************:)
<e/>/(typeswitch (self::node())
case $i as xs:integer return $i
default $v return $v)