(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-41                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure all namespaces in-scope, through a couple of elements, are found by in-scope-prefixes(). :)
(:*******************************************************:)
declare namespace p = "http://www.example.com/A";
"START",
for $i in in-scope-prefixes(<e xmlns="http://www.example.com/A" xmlns:A="http://www.example.com/C">
                                <b xmlns:B="http://www.example.com/C" />
                            </e>/p:b)
order by $i
return $i,
"END"