(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-77                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Pull out a namespace prefix that shadows another. :)
(:*******************************************************:)
declare namespace t = "http://example.com/2";
<p:a xmlns:p="http://example.com/">
    <p:e xmlns:p="http://example.com/2"/>
</p:a>//t:e