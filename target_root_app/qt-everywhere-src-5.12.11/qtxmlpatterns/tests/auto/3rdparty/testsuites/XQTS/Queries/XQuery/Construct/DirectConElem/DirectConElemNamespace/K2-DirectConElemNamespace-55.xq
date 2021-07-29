(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-55                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A combination of computed and direct constructors with namespace undeclarations. :)
(:*******************************************************:)
<e xmlns="http://example.com/">
    <b xmlns="">
        {
        attribute {QName("http://example.com/2", "p:attr")} {()}
        }
    </b>
</e>