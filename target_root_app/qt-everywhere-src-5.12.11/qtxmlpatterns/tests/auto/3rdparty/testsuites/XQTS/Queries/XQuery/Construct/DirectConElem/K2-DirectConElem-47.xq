(:*******************************************************:)
(: Test: K2-DirectConElem-47                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use content that needs to be escaped, inside namespace declaration attributes. :)
(:*******************************************************:)
let $in :=
<r>
        <e xmlns="http://example.com/&lt;&gt;&quot;&apos;"""/>
        <e xmlns='http://example.com/&lt;&gt;&quot;&apos;'''/>
        <p:e xmlns:p="http://example.com/&lt;&gt;&quot;&apos;"""/>
        <p:e xmlns:p='http://example.com/&lt;&gt;&quot;&apos;'''/>
    </r>
return <r>{for $n in $in/*/namespace-uri(.) return <e ns="{$n}"/>}</r>    