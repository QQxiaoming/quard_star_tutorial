(:*******************************************************:)
(: Test: K2-sequenceExprTypeswitch-13                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A complex query that constructs nodes in the wrong order. :)
(:*******************************************************:)
declare variable $i := (<e/>, attribute name {"content"}, <a attr="content"/>, <e/>, 1, "str", <!-- a comment -->);
<d>
    {
        typeswitch(typeswitch($i)
                   case $b as element(e)                return concat("Found an element by name ", string($b))
                   case $b as element()                 return comment{concat("Found: ", string($b))}
                   case $c as attribute(doesntMatch)    return $c/..
                   default $def return $def)
        case $str as xs:string return "A string"
        case $attr as attribute() return string($attr)
        default $def return $def
    }
</d>
