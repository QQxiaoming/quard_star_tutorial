(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-46                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure no declaration is output for the XML namespace. :)
(:*******************************************************:)
let $i := <e xml:space="preserve"/>
return <a>{$i/@*}</a>