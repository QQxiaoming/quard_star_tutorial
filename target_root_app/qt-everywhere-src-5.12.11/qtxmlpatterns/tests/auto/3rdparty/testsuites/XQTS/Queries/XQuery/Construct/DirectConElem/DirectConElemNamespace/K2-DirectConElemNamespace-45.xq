(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-45                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the namespace declaration is output for a copied attribute. :)
(:*******************************************************:)
let $i := <e xmlns:p="http://example.com" p:anAttribute="attrValue"/>
return <a>{$i/@*}</a>