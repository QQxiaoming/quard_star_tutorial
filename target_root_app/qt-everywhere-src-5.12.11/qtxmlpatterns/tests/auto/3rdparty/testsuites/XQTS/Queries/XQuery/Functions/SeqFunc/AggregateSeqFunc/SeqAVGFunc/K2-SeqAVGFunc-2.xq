(:*******************************************************:)
(: Test: K2-SeqAVGFunc-2                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the return type is correct when type promotion is required(#2). :)
(:*******************************************************:)
avg((xs:float('NaN'), 2, 3.3, 4, xs:double('NaN'))) instance of xs:double