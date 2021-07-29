(:*******************************************************:)
(: Test: K2-ComputeConAttr-60                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: 'DEFAULT' is an invalid value for xml:space. :)
(:*******************************************************:)
<a>
                 {
                    attribute xml:space {"DEFAULT"}
                 }
</a>