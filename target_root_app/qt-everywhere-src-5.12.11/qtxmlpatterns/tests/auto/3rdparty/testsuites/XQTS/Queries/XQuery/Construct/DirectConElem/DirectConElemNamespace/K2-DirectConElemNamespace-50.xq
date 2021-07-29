(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-50                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure undeclarations affect path expressions correctly. :)
(:*******************************************************:)
<e xmlns="http://www.example.com/A" xmlns:A="http://www.example.com/C">
<b xmlns:B="http://www.example.com/C" xmlns=""/>
</e>/b