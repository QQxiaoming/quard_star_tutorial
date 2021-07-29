(:*******************************************************:)
(: Test: K2-ComputeConAttr-49                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure XQDY0025 is issued when triggered by computed constructors. :)
(:*******************************************************:)
declare namespace a = "http://example.com/A";
declare namespace b = "http://example.com/A";
<e>
    {
        attribute a:localName {()},
        attribute b:localName {()}
    }
</e>