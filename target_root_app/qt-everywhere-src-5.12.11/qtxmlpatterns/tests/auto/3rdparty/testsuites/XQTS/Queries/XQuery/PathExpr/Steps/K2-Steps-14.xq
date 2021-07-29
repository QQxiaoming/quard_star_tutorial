(:*******************************************************:)
(: Test: K2-Steps-14                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure sorting and de-duplication is applied to variables when appearing in paths. :)
(:*******************************************************:)
<r>
    {
        let $i :=   <e>
                        <a/>
                        <b/>
                    </e>
        let $b := ($i/b, $i/a, $i/b, $i/a)
        return ()/$b
    }
</r>