(:*******************************************************:)
(: Test: K2-ComputeConAttr-59                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: '1' is an invalid value for xml:id.          :)
(:*******************************************************:)
<a>
                 {
                    attribute xml:id {"1"}
                 }
</a>