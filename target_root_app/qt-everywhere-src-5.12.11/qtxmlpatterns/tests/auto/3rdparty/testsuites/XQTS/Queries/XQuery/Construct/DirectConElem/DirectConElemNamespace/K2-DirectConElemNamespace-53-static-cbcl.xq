(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-53                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:32+01:00                       :)
(: Purpose: Ensure a namespace undeclaration aren't counted as a namespace binding. :)
(:*******************************************************:)
count(in-scope-prefixes(exactly-one(<a xmlns="http://example.com/">
    <e xmlns=""/>
</a>/e)))
