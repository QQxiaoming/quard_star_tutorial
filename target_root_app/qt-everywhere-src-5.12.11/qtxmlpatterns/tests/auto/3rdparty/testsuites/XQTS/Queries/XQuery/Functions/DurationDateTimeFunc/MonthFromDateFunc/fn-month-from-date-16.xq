(:*******************************************************:)
(:Test: month-from-date-16                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "month-from-date" function      :)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:month-from-date(xs:date("1970-01-01Z")) eq fn:month-from-date(xs:date("1970-01-01Z"))
