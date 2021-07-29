(:*******************************************************:)
(: Test: K2-sequenceExprTypeswitch-10                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Typeswitch variables are not in scope outside the typeswitch expression. :)
(:*******************************************************:)
typeswitch (1, 2, 3)
case node() return <e/>
case $i as xs:integer return 3
default return 1, $i