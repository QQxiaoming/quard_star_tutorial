(:*******************************************************:)
(: Test: K-SeqExprCast-795                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Verify with 'instance of' that the xs:dateTime constructor function produces values of the correct type. The subsequence() function makes it more difficult for optimizers to take short cuts based on static type information. :)
(:*******************************************************:)

        subsequence(("dummy", 1.1, xs:dateTime("2002-10-10T12:00:00-05:00")), 3, 1) instance of xs:dateTime