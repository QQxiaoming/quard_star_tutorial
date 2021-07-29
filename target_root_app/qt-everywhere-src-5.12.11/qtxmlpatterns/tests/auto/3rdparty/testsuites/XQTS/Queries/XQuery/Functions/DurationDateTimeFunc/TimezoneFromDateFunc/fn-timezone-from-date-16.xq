(:*******************************************************:)
(:Test: timezone-from-date-16                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-date" function   :)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:timezone-from-date(xs:date("1970-01-02+10:00")) eq fn:timezone-from-date(xs:date("1970-01-01+10:00"))
