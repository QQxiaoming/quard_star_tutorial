(:*******************************************************:)
(: Test: K2-ConDocNode-26                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Attributes cannot be children of document nodes. :)
(:*******************************************************:)
<doo>
    {
        document
        {
            attribute name {"content"}
        }
    }
</doo>