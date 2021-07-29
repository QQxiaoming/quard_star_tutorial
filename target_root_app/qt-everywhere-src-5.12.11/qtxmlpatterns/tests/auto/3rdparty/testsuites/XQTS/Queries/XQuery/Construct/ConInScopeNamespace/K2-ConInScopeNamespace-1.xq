(:*******************************************************:)
(: Test: K2-ConInScopeNamespace-1                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the in-scope prefixes are correct for all top-level elements. :)
(:*******************************************************:)
let $i := document{<e1/>, <e2/>, <e3/>, <e4/>}
return (in-scope-prefixes($i/e1),
        in-scope-prefixes($i/e2),
        in-scope-prefixes($i/e3),
        in-scope-prefixes($i/e4))