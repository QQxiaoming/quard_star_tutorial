(:*******************************************************:)
(:Test: timezone-from-date-17                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "month-from-date" function      :)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:month-from-date(xs:date("1970-01-01+05:00")) ne fn:month-from-date(xs:date("1970-01-03+03:00"))
