import React, { Component } from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { Button, Modal } from 'antd'
import I18N from '@/I18N'
import BudgetForm from '@/module/form/BudgetForm/Container'
import PaymentList from './PaymentList'
import _ from 'lodash'

class PaymentSchedule extends Component {
  constructor(props) {
    super(props)
    const value = props.initialValue
    this.state = {
      visible: false,
      total: (value && value.budgetAmount) || '',
      address: (value && value.elaAddress) || '',
      paymentItems: (value && value.paymentItems) || [],
      errors: {}
    }
  }

  changeValue(value) {
    const { onChange, callback } = this.props
    onChange(value)
    callback('budget')
  }

  passDataToParent() {
    const { total, address, paymentItems } = this.state
    this.changeValue({
      budgetAmount: Number(total),
      elaAddress: address,
      paymentItems
    })
  }

  validateAmount = (value) => {
    const reg = /^(0|[1-9][0-9]*)(\.[0-9]*)?$/
    return (!isNaN(value) && reg.test(value)) || value === '' ? true : false
  }

  validateAddress = (value) => {
    const reg = /^[E8][a-zA-Z0-9]+$/
    const len = value.length === 34
    return !value || (reg.test(value) && len)
  }

  validateFields = (field, value) => {
    const { errors } = this.state
    if (field === 'total' && !this.validateAmount(value)) {
      return {
        ...errors,
        [field]: I18N.get('suggestion.form.error.isNaN')
      }
    }
    if (field === 'address' && !this.validateAddress(value)) {
      return {
        ...errors,
        [field]: I18N.get('suggestion.form.error.elaAddress')
      }
    }
    return { ...errors, [field]: '' }
  }

  handleChange = (e, field) => {
    const errors = this.validateFields(field, e.target.value)
    this.setState({ [field]: e.target.value, errors }, () => {
      this.passDataToParent()
    })
  }

  hideModal = () => {
    this.setState({ visible: false })
  }

  showModal = () => {
    this.setState({ visible: true, index: -1 })
  }

  sortPayments = (arr) => {
    return _.sortBy(arr, (item) => parseInt(item.milestoneKey, 10))
  }

  handleDelete = (index) => {
    const { paymentItems } = this.state
    const rs = [
      ...paymentItems.slice(0, index),
      ...paymentItems.slice(index + 1)
    ]
    const sortedItems = this.sortPayments(rs)
    const errors = this.validateFields('schedule', sortedItems)
    this.setState(
      {
        paymentItems: sortedItems,
        errors
      },
      () => {
        this.passDataToParent()
      }
    )
  }

  handleEdit = (index) => {
    this.setState({ index, visible: true })
  }

  handleSubmit = (values) => {
    const { paymentItems, index } = this.state
    if (index >= 0) {
      const rs = paymentItems.map((item, key) => {
        if (index === key) {
          return values
        }
        return item
      })
      const sortedItems = this.sortPayments(rs)
      this.setState({ paymentItems: sortedItems, visible: false }, () => {
        this.passDataToParent()
      })
      return
    }
    const rs = [...paymentItems, values]
    const sortedItems = this.sortPayments(rs)
    const errors = this.validateFields('schedule', sortedItems)
    this.setState(
      {
        paymentItems: sortedItems,
        visible: false,
        errors
      },
      () => {
        this.passDataToParent()
      }
    )
  }

  getMilestone = () => {
    const milestone = sessionStorage.getItem('plan-milestone') || []
    try {
      const rs = JSON.parse(milestone)
      return Array.isArray(rs) ? rs : []
    } catch (err) {
      return []
    }
  }

  render() {
    const { paymentItems, index, total, address, errors } = this.state
    const milestone = this.getMilestone()
    const flag = milestone && milestone.length <= paymentItems.length
    const disabled = !milestone || flag
    return (
      <Wrapper>
        <Section>
          <Label>{`${I18N.get('suggestion.budget.total')} (ELA)`}</Label>
          <StyledInput
            error={errors.total ? true : false}
            value={total}
            onChange={(e) => this.handleChange(e, 'total')}
          />
          {errors.total ? <Error>{errors.total}</Error> : null}
        </Section>
        <Section>
          <Label>{I18N.get('suggestion.budget.address')}</Label>
          <StyledInput
            error={errors.address ? true : false}
            value={address}
            onChange={(e) => this.handleChange(e, 'address')}
          />
          {errors.address ? <Error>{errors.address}</Error> : null}
        </Section>
        <Section>
          <Label>{I18N.get('suggestion.budget.schedule')}</Label>
          <Button onClick={this.showModal} disabled={disabled ? true : false}>
            {I18N.get('suggestion.budget.create')}
          </Button>
          {disabled ? <Tip>{I18N.get('suggestion.budget.tip')}</Tip> : null}
        </Section>
        {errors.schedule ? <Error>{errors.schedule}</Error> : null}
        {paymentItems.length ? (
          <PaymentList
            list={paymentItems}
            milestone={milestone}
            onDelete={this.handleDelete}
            onEdit={this.handleEdit}
          />
        ) : null}
        <Modal
          maskClosable={false}
          visible={this.state.visible}
          onCancel={this.hideModal}
          footer={null}
          width={770}
        >
          {this.state.visible === true ? (
            <BudgetForm
              item={index >= 0 ? paymentItems[index] : null}
              types={paymentItems.map((item) => item.type)}
              onSubmit={this.handleSubmit}
              onCancel={this.hideModal}
              milestone={milestone}
              total={total}
              paymentItems={paymentItems}
            />
          ) : null}
        </Modal>
      </Wrapper>
    )
  }
}

PaymentSchedule.propTypes = {
  onChange: PropTypes.func,
  callback: PropTypes.func,
  initialValue: PropTypes.object
}

export default PaymentSchedule

const Wrapper = styled.div``

const Label = styled.div`
  font-size: 17px;
  line-height: 24px;
  color: #000000;
`
const Section = styled.div`
  margin-top: 24px;
  .ant-btn {
    margin-top: 16px;
    border: 1px solid #000000;
    color: #000000;
    &:hover {
      border: 1px solid #008d85;
      color: #008d85;
    }
  }
`
const StyledInput = styled.input`
  outline: 0;
  background: #ffffff;
  border: 1px solid #d9d9d9;
  margin-top: 16px;
  padding: 6px 11px;
  width: 50%;
  height: 40px;
  transition: all 0.3s;
  &:hover {
    border-color: ${(props) => (props.error ? '#f5222d' : '#66bda3')};
  }
  &:focus {
    border-color: ${(props) => (props.error ? '#f5222d' : '#66bda3')};
    box-shadow: ${(props) =>
      props.error
        ? '0 0 0 2px rgba(245, 34, 45, 0.2);'
        : '0 0 0 2px rgba(67, 175, 146, 0.2)'};
  }
`
const Error = styled.div`
  color: #f5222d;
`
const Tip = styled.div`
  color: #666;
  font-size: 13px;
`
