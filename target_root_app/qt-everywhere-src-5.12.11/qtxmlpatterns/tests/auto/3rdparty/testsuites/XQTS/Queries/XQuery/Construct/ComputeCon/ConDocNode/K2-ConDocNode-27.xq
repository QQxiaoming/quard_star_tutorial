(:*******************************************************:)
(: Test: K2-ConDocNode-27                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Attributes cannot be children of document nodes(#2). :)
(:*******************************************************:)
<doo>
    {
        document
        {
            <e/>,
            attribute name {"content"}
        }
    }
</doo>