(:*******************************************************:)
(:Test: timezone-from-date-9                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-date" function   :)
(:as part of a "-" expression.                           :) 
(:*******************************************************:)

fn:timezone-from-date(xs:date("1970-01-01+09:00")) - fn:timezone-from-date(xs:date("1970-01-01+10:00"))
