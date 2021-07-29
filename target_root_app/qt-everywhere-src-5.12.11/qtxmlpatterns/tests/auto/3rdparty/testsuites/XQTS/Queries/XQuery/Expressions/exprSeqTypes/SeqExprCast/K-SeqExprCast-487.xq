(:*******************************************************:)
(: Test: K-SeqExprCast-487                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Verify with 'instance of' that the xs:double constructor function produces values of the correct type. The subsequence() function makes it more difficult for optimizers to take short cuts based on static type information. :)
(:*******************************************************:)

        subsequence(("dummy", 1.1, xs:double("3.3e3")), 3, 1) instance of xs:double