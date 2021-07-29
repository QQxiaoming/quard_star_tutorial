(:*******************************************************:)
(: Test: K-ErrorFunc-2                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `if(true()) then true() else error(QName("", "local"), "description")`. :)
(:*******************************************************:)
if(true()) then true() else 
						error(QName("", "local"), "description")