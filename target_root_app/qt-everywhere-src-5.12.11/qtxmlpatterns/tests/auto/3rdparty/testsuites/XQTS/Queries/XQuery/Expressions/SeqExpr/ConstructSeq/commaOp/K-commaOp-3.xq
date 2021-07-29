(:*******************************************************:)
(: Test: K-commaOp-3                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: An expression sequence containing many empty sequences and one xs:string. On some implementations this triggers certain optimization paths. :)
(:*******************************************************:)
((), (), ((), (), ((), (), ("str")), ()), (), (())) eq "str"