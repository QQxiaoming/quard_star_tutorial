(:*******************************************************:)
(: Test: K2-ConDocNode-29                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An invalid attribute constructor as child to the document constructor. :)
(:*******************************************************:)
<doo>
    {
        document
        {
            <e>
                {
                    <?target data?>,
                    attribute name {"content"}
                }
            </e>
        }
    }
</doo>