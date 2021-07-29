(:*******************************************************:)
(: Test: K2-ConDocNode-28                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Attributes cannot be children of document nodes(#3). :)
(:*******************************************************:)
<doo>
    {
        document
        {
            <e>
                <b/>
                <b/>
                <b/>
                <c>
                    <d/>
                </c>
            </e>,
            attribute name {"content"}
        }
    }
</doo>