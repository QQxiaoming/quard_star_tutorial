(:*******************************************************:)
(:Test: timezone-from-date-11                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-date" function   :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:timezone-from-date(xs:date("1970-01-02+10:00")) div fn:timezone-from-date(xs:date("1970-01-01+05:00"))
