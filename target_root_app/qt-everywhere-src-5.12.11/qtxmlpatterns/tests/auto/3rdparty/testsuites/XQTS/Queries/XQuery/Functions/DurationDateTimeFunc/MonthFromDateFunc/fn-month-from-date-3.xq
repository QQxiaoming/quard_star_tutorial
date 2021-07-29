(:*******************************************************:)
(:Test: month-from-date-3                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "month-from-date" function      :)
(:involving a "numeric-less-than" operation (lt operator):)
(:*******************************************************:)

fn:month-from-date(xs:date("1999-12-31Z")) lt fn:month-from-date(xs:date("1999-12-31Z"))