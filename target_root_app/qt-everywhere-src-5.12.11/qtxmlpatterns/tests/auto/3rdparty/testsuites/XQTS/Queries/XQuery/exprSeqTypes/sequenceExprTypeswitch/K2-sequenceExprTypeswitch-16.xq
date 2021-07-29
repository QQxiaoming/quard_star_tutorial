(:*******************************************************:)
(: Test: K2-sequenceExprTypeswitch-16                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use variables with type declarations.        :)
(:*******************************************************:)
typeswitch(<e/>, <e/>)
case $b as element() return concat("", $b treat as element())
default return 1