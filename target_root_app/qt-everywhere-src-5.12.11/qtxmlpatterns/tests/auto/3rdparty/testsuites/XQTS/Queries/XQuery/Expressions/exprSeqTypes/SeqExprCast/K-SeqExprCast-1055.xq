(:*******************************************************:)
(: Test: K-SeqExprCast-1055                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Verify with 'instance of' that the xs:gMonthDay constructor function produces values of the correct type. The subsequence() function makes it more difficult for optimizers to take short cuts based on static type information. :)
(:*******************************************************:)

        subsequence(("dummy", 1.1, xs:gMonthDay("--11-13")), 3, 1) instance of xs:gMonthDay