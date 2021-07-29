(:*******************************************************:)
(: Test: K-CombinedErrorCodes-7                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: ':=' cannot be used to assing namespaces in 'import schema'. :)
(:*******************************************************:)
import schema namespace NCName := "http://example.com/Dummy"; 1