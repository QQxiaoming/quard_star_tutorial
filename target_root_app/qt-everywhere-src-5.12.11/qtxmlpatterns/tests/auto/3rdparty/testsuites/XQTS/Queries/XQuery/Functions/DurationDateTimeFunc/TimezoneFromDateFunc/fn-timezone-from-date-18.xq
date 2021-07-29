(:*******************************************************:)
(:Test: timezone-from-date-18                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-date" function   :)
(:as part of a "numeric-equal" expression (le operator)  :) 
(:*******************************************************:)

fn:timezone-from-date(xs:date("1970-01-01+04:00")) le fn:timezone-from-date(xs:date("1970-01-02+02:00"))