import React from 'react'
import ReactDOMServer from 'react-dom/server'
import _ from 'lodash'
import { Row, Col, Spin, Modal, Input, Button, Anchor } from 'antd'
import { Link } from 'react-router-dom'
import MediaQuery from 'react-responsive'
import moment from 'moment/moment'
import Comments from '@/module/common/comments/Container'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
import Translation from '@/module/common/Translation/Container'
import SuggestionForm from '@/module/form/SuggestionForm/Container'
import I18N from '@/I18N'
import { LG_WIDTH } from '@/config/constant'
import { CVOTE_STATUS, SUGGESTION_TAG_TYPE, CONTENT_TYPE } from '@/constant'
import { getSafeUrl } from '@/util/url'
import sanitizeHtml from '@/util/html'
import { ReactComponent as CommentIcon } from '@/assets/images/icon-info.svg'
import StandardPage from '../../StandardPage'
import ActionsContainer from '../common/actions/Container'
import MetaContainer from '../common/meta/Container'
import Meta from '@/module/common/Meta'
import SocialShareButtons from '@/module/common/SocialShareButtons'
import DraftEditor from '@/module/common/DraftEditor'
import {
  Container,
  Title,
  CoverImg,
  ShortDesc,
  DescLabel,
  Label,
  LabelPointer,
  Desc,
  BtnGroup,
  StyledButton,
  DescBody,
  CouncilComments,
  IconWrap,
  Item,
  ItemTitle,
  ItemText,
  StyledAnchor
} from './style'

import './style.scss'

const { TextArea } = Input

export default class extends StandardPage {
  constructor(props) {
    super(props)

    // we use the props from the redux store if its retained
    this.state = {
      isDropdownActionOpen: false,
      showMobile: false,
      showForm: false,
      needsInfoVisible: false,
      proposeLoading: false
    }
  }

  componentDidMount() {
    super.componentDidMount()
    this.refetch(true)
  }

  componentWillUnmount() {
    this.props.resetDetail()
  }

  renderAnchors() {
    return (
      <StyledAnchor>
        <Anchor.Link
          href="#preamble"
          title={I18N.get('suggestion.fields.preamble')}
        />
        <Anchor.Link
          href="#abstract"
          title={I18N.get('suggestion.fields.abstract')}
        />
        <div style={{ marginTop: 48 }}>
          <Anchor.Link href="#goal" title={I18N.get('suggestion.fields.goal')} />
        </div>
        <Anchor.Link
          href="#motivation"
          title={I18N.get('suggestion.fields.motivation')}
        />
        <Anchor.Link href="#plan" title={I18N.get('suggestion.fields.plan')} />
        <Anchor.Link
          href="#relevance"
          title={I18N.get('suggestion.fields.relevance')}
        />
        <div style={{ marginTop: 48 }}>
          <Anchor.Link
            href="#budget"
            title={I18N.get('suggestion.fields.budget')}
          />
        </div>
      </StyledAnchor>
    )
  }
  ord_renderContent() {
    const { detail } = this.props
    if (_.isEmpty(detail) || detail.loading) {
      return (
        <div className="center">
          <Spin size="large" />
        </div>
      )
    }
    const detailNode = this.renderDetail()
    const translationBtn = this.renderTranslationBtn()
    const actionsNode = this.renderActionsNode()
    const ownerActionsNode = this.renderOwnerActionsNode()
    const councilActionsNode = this.renderCouncilActionsNode()
    const editForm = this.renderEditForm()
    // const mySuggestionNode = <MySuggestion />
    const commentNode = this.renderCommentNode()
    const socialShareButtonsNode = this.renderSocialShareButtonsNode()

    return (
      <div>
        <Meta
          desc={detail.shortDesc}
          title={`${detail.title} - Suggestion Detail - Cyber Republic`}
          url={this.props.location.pathname}
        />

        <Container className="c_SuggestionDetail">
          <MediaQuery maxWidth={LG_WIDTH}>
            <div>
              <BackLink
                link="/suggestion"
                style={{ position: 'relative', left: 0, marginBottom: 15 }}
              />
              {this.renderAnchors()}
            </div>
            <div>
              {detailNode}
              {translationBtn}
              {socialShareButtonsNode}
              {actionsNode}
              {ownerActionsNode}
              {councilActionsNode}
            </div>
            {/* <div>{mySuggestionNode}</div> */}
            <div style={{ marginTop: 60 }}>{commentNode}</div>
          </MediaQuery>
          <MediaQuery minWidth={LG_WIDTH + 1}>
            <BackLink link="/suggestion" />
            {this.renderAnchors()}
            <Row gutter={24}>
              <Col span={24}>
                {detailNode}
                {translationBtn}
                {socialShareButtonsNode}
                {actionsNode}
                {ownerActionsNode}
                {councilActionsNode}
                <div style={{ marginTop: 60 }}>{commentNode}</div>
              </Col>
              {/* <Col span={9}>{mySuggestionNode}</Col> */}
            </Row>
          </MediaQuery>
          {editForm}
        </Container>
        <Footer />
      </div>
    )
  }

  renderPreambleItem(header, content) {
    return (
      <Item>
        <Col span={6}>
          <ItemTitle>{header}</ItemTitle>
        </Col>
        <Col span={18}>
          <ItemText>{content}</ItemText>
        </Col>
      </Item>
    )
  }

  renderDetail() {
    const { detail } = this.props

    const metaNode = this.renderMetaNode()
    const titleNode = this.renderTitleNode()
    const coverNode = this.renderCoverNode()
    const shortDescNode = this.renderShortDescNode()
    // const labelNode = this.renderLabelNode()
    const tagsNode = this.renderTagsNode()
    const descNode = this.renderDescNode()
    const benefitsNode = this.renderBenefitsNode()
    const fundingNode = this.renderFundingNode()
    const timelineNode = this.renderTimelineNode()
    const linkNode = this.renderLinkNode()
    return (
      <div>
        {metaNode}
        {titleNode}
        {tagsNode}

        <DescLabel id="preamble">{I18N.get('suggestion.fields.preamble')}</DescLabel>
        {this.renderPreambleItem(I18N.get('proposal.fields.preambleSub.proposal'), `#${detail.displayId}`)}
        {this.renderPreambleItem(I18N.get('proposal.fields.preambleSub.title'), detail.title)}
        {this.renderPreambleItem(I18N.get('proposal.fields.preambleSub.proposer'), detail.createdBy.username)}
        {this.renderPreambleItem(I18N.get('proposal.fields.preambleSub.status'), detail.status)}
        {this.renderPreambleItem(I18N.get('proposal.fields.preambleSub.created'), moment(detail.createdAt).format('MMM D, YYYY'))}

        <DescLabel id="abstract">{I18N.get('suggestion.fields.abstract')}</DescLabel>
        <DraftEditor value={detail.abstract} editorEnabled={false} contentType={CONTENT_TYPE.MARKDOWN} />
        <DescLabel id="goal">{I18N.get('suggestion.fields.goal')}</DescLabel>
        <DraftEditor value={detail.goal} editorEnabled={false} contentType={CONTENT_TYPE.MARKDOWN} />
        <DescLabel id="motivation">{I18N.get('suggestion.fields.motivation')}</DescLabel>
        <DraftEditor value={detail.motivation} editorEnabled={false} contentType={CONTENT_TYPE.MARKDOWN} />
        <DescLabel id="plan">{I18N.get('suggestion.fields.plan')}</DescLabel>
        <DraftEditor value={detail.plan} editorEnabled={false} contentType={CONTENT_TYPE.MARKDOWN} />
        <DescLabel id="relevance">{I18N.get('suggestion.fields.relevance')}</DescLabel>
        <DraftEditor value={detail.relevance} editorEnabled={false} contentType={CONTENT_TYPE.MARKDOWN} />
        <DescLabel id="budget">{I18N.get('suggestion.fields.budget')}</DescLabel>
        <DraftEditor value={detail.budget} editorEnabled={false} contentType={CONTENT_TYPE.MARKDOWN} />
        {/* {coverNode} */}
        {/* {shortDescNode} */}
        {/* <Divider /> */}
        {/* {descNode} */}
        {/* <Divider /> */}
        {/* {benefitsNode} */}
        {/* {fundingNode} */}
        {/* {timelineNode} */}
        {/* {linkNode} */}
      </div>
    )
  }

  renderSocialShareButtonsNode() {
    const { detail } = this.props
    return (
      <SocialShareButtons
        shareQuote={`${detail.title} - Suggestion Detail - Cyber Republic`}
      />
    )
  }

  renderMetaNode() {
    const { detail } = this.props
    return <MetaContainer data={detail} />
  }

  renderTitleNode() {
    const { detail } = this.props
    return <Title>{detail.title}</Title>
  }

  renderCoverNode() {
    const { detail } = this.props
    return detail.coverImg ? <CoverImg src={detail.coverImg} /> : ''
  }

  renderShortDescNode() {
    const { detail } = this.props
    return <ShortDesc>{detail.shortDesc}</ShortDesc>
  }

  renderLabelNode() {
    const reference = _.get(this.props.detail, 'reference')
    if (_.isEmpty(reference)) return null
    const { _id, vid, status } = _.last(reference)
    // when proposal is draft, do not show the label
    if (status === CVOTE_STATUS.DRAFT) return null
    const linkText = `${I18N.get('council.voting.proposal')} #${vid}`
    return (
      <Label>
        {`${I18N.get('suggestion.referred')} `}
        <Link to={`/proposals/${_id}`}>{linkText}</Link>
        {` (${I18N.get(`cvoteStatus.${status}`)})`}
      </Label>
    )
  }

  renderTagsNode() {
    const tags = _.get(this.props.detail, 'tags')
    if (_.isEmpty(tags)) return null
    const res = _.map(tags, tag => {
      const { type, _id, desc } = tag
      return (
        <div key={_id}>
          {type === SUGGESTION_TAG_TYPE.INFO_NEEDED && (
            <LabelPointer
              type={type}
              data-desc={desc.replace(/(['"])/g, '\\$1')}
              onClick={() => this.setState({ needsInfoVisible: true })}
            >
              {I18N.get(`suggestion.tag.type.${type}`)}
              {' '}
&nbsp;
              <IconWrap>
                <CommentIcon className="more-info-icon" />
              </IconWrap>
            </LabelPointer>
          )}
          <Modal
            title={I18N.get(`suggestion.tag.type.${type}`)}
            visible={this.state.needsInfoVisible}
            onCancel={this.closeNeedsInfoModal.bind(this)}
            footer={[
              <Button key="close" onClick={this.closeNeedsInfoModal.bind(this)}>
                Close
              </Button>
            ]}
          >
            <div style={{ fontWeight: 200, paddingBottom: '18px' }}>
              {I18N.get('suggestion.modal.pleaseUpdate')}
            </div>
            {I18N.get('suggestion.modal.commentsFromCouncil')}
            <br />
            <CouncilComments>{desc}</CouncilComments>
          </Modal>
        </div>
      )
    })
    return res
  }

  closeNeedsInfoModal() {
    this.setState({
      needsInfoVisible: false
    })
  }

  renderDescNode() {
    const { detail } = this.props
    return (
      <Desc>
        <DescLabel>{I18N.get('suggestion.form.fields.fullDesc')}</DescLabel>
        <DescBody
          className="ql-editor"
          dangerouslySetInnerHTML={{ __html: sanitizeHtml(detail.desc) }}
        />
      </Desc>
    )
  }

  renderBenefitsNode() {
    const { detail } = this.props
    if (!detail.benefits) {
      return null
    }
    return (
      <Desc>
        <DescLabel>{I18N.get('suggestion.form.fields.benefits')}</DescLabel>
        <DescBody>{detail.benefits}</DescBody>
      </Desc>
    )
  }

  renderFundingNode() {
    const { detail } = this.props
    if (!detail.funding) {
      return null
    }
    return (
      <Desc>
        <DescLabel>{I18N.get('suggestion.form.fields.funding')}</DescLabel>
        <div>{detail.funding}</div>
      </Desc>
    )
  }

  renderTimelineNode() {
    const { detail } = this.props
    if (!detail.timeline) {
      return null
    }
    return (
      <Desc>
        <DescLabel>{I18N.get('suggestion.form.fields.timeline')}</DescLabel>
        <div>{moment(detail.timeline).format('MMM D, YYYY')}</div>
      </Desc>
    )
  }

  renderLinkNode() {
    const { link } = this.props.detail

    if (_.isEmpty(link || _.isEmpty(_.get(link, '[0]')))) {
      return null
    }

    return (
      <Desc>
        <DescLabel>{I18N.get('suggestion.form.fields.links')}</DescLabel>
        {_.map(link, href => (
          <div key={href}>
            <a
              href={getSafeUrl(href)}
              target="_blank"
              rel="noopener noreferrer"
            >
              {href}
            </a>
          </div>
        ))}
      </Desc>
    )
  }

  renderTranslationBtn() {
    const { title, desc, benefits } = this.props.detail
    const text = `
      <h1>${title}</h1>
      <h4>${I18N.get('suggestion.form.fields.desc')}</h4>
      ${desc}
      ${
  benefits
    ? `<h4>${I18N.get('suggestion.form.fields.benefits')}</h4>
      <p>${benefits}</p>`
    : ''
}
    `

    return (
      <div style={{ marginTop: 20 }}>
        <Translation text={text} />
      </div>
    )
  }

  renderActionsNode() {
    const { detail } = this.props
    return <ActionsContainer data={detail} />
  }

  onCreated = () => {
    this.refetch()
    this.props.history.push('/proposals')
  }

  renderOwnerActionsNode() {
    const { detail, currentUserId, isAdmin } = this.props
    const isOwner = currentUserId === _.get(detail, 'createdBy._id') || isAdmin
    const res = isOwner && (
      <StyledButton
        type="ebp"
        className="cr-btn cr-btn-default"
        onClick={this.showEditForm}
      >
        {I18N.get('suggestion.btnText.edit')}
      </StyledButton>
    )
    return res
  }

  renderCouncilActionsNode() {
    // const { isCouncil, match } = this.props

    // return isCouncil && (
    //   <StyledButton
    //     type="ebp"
    //     className="cr-btn cr-btn-default"
    //     onClick={this.}
    //   >
    //     {I18N.get('suggestion.btnText.edit')}
    //   </StyledButton>
    // )

    const { isCouncil, isAdmin, detail } = this.props
    const { _id, displayId, title } = detail
    const descNode = this.renderDescNode()
    const benefitsNode = this.renderBenefitsNode()
    const fundingNode = this.renderFundingNode()
    const timelineNode = this.renderTimelineNode()
    const linkNode = this.renderLinkNode()

    const proposalContent = `
      ${ReactDOMServer.renderToString(descNode)}
      ${ReactDOMServer.renderToString(benefitsNode)}
      ${ReactDOMServer.renderToString(fundingNode)}
      ${ReactDOMServer.renderToString(timelineNode)}
      ${ReactDOMServer.renderToString(linkNode)}
    `

    const props = {
      data: {
        title,
        content: proposalContent
      },
      suggestionDisplayId: displayId,
      suggestionId: _id,
      onCreated: this.onCreated,
      btnText: I18N.get('suggestion.btnText.makeIntoProposal'),
      btnStyle: { width: 200 }
    }
    const considerBtn = (isCouncil || isAdmin) && (
      <Col xs={24} sm={8}>
        <StyledButton
          type="ebp"
          className="cr-btn cr-btn-default"
          onClick={this.consider}
        >
          {I18N.get('suggestion.btnText.markConsider')}
        </StyledButton>
      </Col>
    )
    const needMoreInfoBtn = (isCouncil || isAdmin) && (
      <Col xs={24} sm={8}>
        <StyledButton
          type="ebp"
          className="cr-btn cr-btn-default"
          onClick={this.showAddTagModal}
        >
          {I18N.get('suggestion.btnText.needMoreInfo')}
        </StyledButton>
      </Col>
    )
    const createFormBtn = isCouncil && (
      <Col xs={24} sm={8}>
        <StyledButton
          type="ebp"
          className="cr-btn cr-btn-default"
          disabled={this.state.proposeLoading}
          onClick={this.makeIntoPropose}
        >
          {I18N.get('suggestion.btn.makeIntoProposal')}
        </StyledButton>
      </Col>
    )

    const res = (
      <BtnGroup>
        <Row type="flex" justify="start">
          {considerBtn}
          {needMoreInfoBtn}
          {createFormBtn}
        </Row>
      </BtnGroup>
    )
    return res
  }

  renderCommentNode() {
    const { detail } = this.props
    return (
      <Comments
        type="suggestion"
        suggestion={detail}
        canPost={true}
        model={detail._id}
        returnUrl={`/suggestion/${detail._id}`}
      />
    )
  }

  consider = async () => {
    const { _id } = this.props.detail
    try {
      await this.props.addTag({
        id: _id,
        type: SUGGESTION_TAG_TYPE.UNDER_CONSIDERATION
      })
      this.refetch()
    } catch (error) {
      // console.log(error)
    }
  }

  needMoreInfo = async () => {
    const { comment } = this.state
    const { _id } = this.props.detail
    try {
      await this.props.addTag({
        id: _id,
        type: SUGGESTION_TAG_TYPE.INFO_NEEDED,
        desc: comment
      })
      // this.showAddTagModal()
      this.refetch()
    } catch (error) {
      // console.log(error)
    }
  }

  showAddTagModal = () => {
    Modal.confirm({
      title: I18N.get('suggestion.modal.addTagComment'),
      content: <TextArea onChange={this.onCommentChanged} />,
      okText: I18N.get('suggestion.modal.confirm'),
      cancelText: I18N.get('suggestion.modal.cancel'),
      onOk: () => this.needMoreInfo()
    })
  }

  onCommentChanged = e => {
    this.setState({ comment: e.target.value })
  }

  onFormSubmit = async param => {
    try {
      await this.props.update(param)
      this.showEditForm()
      this.refetch()
    } catch (error) {
      // console.log(error)
    }
  }

  renderEditForm = () => {
    const { detail } = this.props

    const props = {
      onFormCancel: this.showEditForm,
      onFormSubmit: this.onFormSubmit,
      header: I18N.get('suggestion.header.edit'),
      data: detail
    }

    return (
      <Modal
        className="project-detail-nobar"
        maskClosable={false}
        visible={this.state.showForm}
        onOk={this.showEditForm}
        onCancel={this.showEditForm}
        footer={null}
        width="70%"
      >
        {this.state.showForm && <SuggestionForm {...props} />}
      </Modal>
    )
  }

  showEditForm = () => {
    const id = _.get(this.props, 'match.params.id')
    this.props.history.push(`/suggestion/${id}/edit`)
    // this.setState({
    //   showForm: !showForm
    // })
  }

  showDropdownActions = () => {
    const { isDropdownActionOpen } = this.state
    this.setState({
      isDropdownActionOpen: !isDropdownActionOpen
    })
  }

  refetch = async incViewsNum => {
    const id = _.get(this.props, 'match.params.id')
    await this.props.resetDetail()
    this.props.getDetail({ id, incViewsNum: !!incViewsNum })
  }

  linkSuggestionDetail(suggestionId) {
    this.props.history.push(`/suggestion/${suggestionId}`)
  }

  makeIntoPropose = async () => {
    const id = _.get(this.props, 'match.params.id')
    const { current_user_id, profile, history } = this.props.user
    const fullName = `${profile.firstName} ${profile.lastName}`
    const { proposeSuggestion } = this.props

    const param = {
      proposedBy: fullName,
      proposer: current_user_id,
      suggestionId: id
    }

    this.setState({ proposeLoading: true })

    try {
      const res = await proposeSuggestion(param)
      this.props.history.push(`/proposals/${res._id}`)
    } catch (error) {
      this.setState({ proposeLoading: false })
    }
  }
}
