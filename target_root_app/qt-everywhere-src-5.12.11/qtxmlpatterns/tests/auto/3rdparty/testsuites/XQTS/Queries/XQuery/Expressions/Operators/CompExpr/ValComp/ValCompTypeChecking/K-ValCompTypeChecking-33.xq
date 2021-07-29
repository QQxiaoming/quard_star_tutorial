(:*******************************************************:)
(: Test: K-ValCompTypeChecking-33                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An expression involving the ne operator that trigger certain optimization paths in some implementations. :)
(:*******************************************************:)
count((0, current-time())) ne 0