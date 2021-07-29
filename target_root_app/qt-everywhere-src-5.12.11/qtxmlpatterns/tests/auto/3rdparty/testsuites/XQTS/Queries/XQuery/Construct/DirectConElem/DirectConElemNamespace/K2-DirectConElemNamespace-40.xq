(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-40                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the namespaces in-scope are outputted. :)
(:*******************************************************:)
declare namespace p = "http://www.example.com/A";
<e xmlns="http://www.example.com/A" xmlns:A="http://www.example.com/C">
    <b xmlns:B="http://www.example.com/C"/>
</e>/p:b