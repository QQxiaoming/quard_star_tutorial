(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-3                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Element constructors aren't well-formed despite the namespace declarations. :)
(:*******************************************************:)
<a:elem xmlns:a="http://example.com/NS" xmlns:b="http://example.com/NS"></b:elem>
    declare default element namespace "http://example.com/NS";