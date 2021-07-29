(:*******************************************************:)
(:Test: timezone-from-date-19                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-date" function   :)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:timezone-from-date(xs:date("1970-01-03+02:00")) ge fn:timezone-from-date(xs:date("1970-01-01+01:00"))