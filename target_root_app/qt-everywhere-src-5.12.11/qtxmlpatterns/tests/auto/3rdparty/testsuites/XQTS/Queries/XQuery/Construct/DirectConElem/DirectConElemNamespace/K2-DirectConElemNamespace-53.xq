(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-53                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure a namespace undeclaration aren't counted as a namespace binding. :)
(:*******************************************************:)
count(in-scope-prefixes(<a xmlns="http://example.com/">
    <e xmlns=""/>
</a>/e))