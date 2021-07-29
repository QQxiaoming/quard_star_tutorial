(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-8                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: It is ok to have space between '$' and the name in variable names. :)
(:*******************************************************:)
declare variable $


                         name := 3;
                         $


                                
                                name
